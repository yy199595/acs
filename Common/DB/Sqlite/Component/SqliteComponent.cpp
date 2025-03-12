#include"SqliteComponent.h"
#include "XCode/XCode.h"
#include"Server/Config/ServerConfig.h"
#include"Util/File/DirectoryHelper.h"
#include "Util/File/FileHelper.h"
#include"Util/Sql/SqlHelper.h"
#include "Lua/Lib/Lib.h"
#include "Util/Tools/TimeHelper.h"
constexpr const char* LOCAL_DATA = "local_data";

namespace acs
{
    SqliteComponent::SqliteComponent()
        : mDatabase(nullptr)
    {
        REGISTER_JSON_CLASS_FIELD(sqlite::Config, script);
        REGISTER_JSON_CLASS_MUST_FIELD(sqlite::Config, db);
        REGISTER_JSON_CLASS_MUST_FIELD(sqlite::Config, dir);
    }

    bool SqliteComponent::Awake()
    {
        LuaCCModuleRegister::Add([](Lua::CCModule& ccModule)
        {
            ccModule.Open("db.sqlite", lua::lib::luaopen_lsqlitedb);
        });
        LOG_CHECK_RET_FALSE(ServerConfig::Inst()->Get("sqlite", this->mConfig));
        return true;
    }

    bool SqliteComponent::LateAwake()
    {
        std::string dir = this->mConfig.dir;
        if (!help::dir::DirectorIsExist(dir))
        {
            help::dir::MakeDir(dir);
        }
        std::string path = fmt::format("{0}/{1}.db", dir, this->mConfig.db);
        if (sqlite3_open(path.c_str(), &this->mDatabase) != SQLITE_OK)
        {
            LOG_ERROR("open sqlite db {} fail", this->mConfig.db);
            return false;
        }
        std::string sql = fmt::format("CREATE TABLE IF NOT EXISTS {}"
                                      "({} {} NOT NULL UNIQUE,"
                                      "{} {} NOT NULL,"
                                      "{} {} NOT NULL,"
                                      "{} {} DEFAULT 0);",
                                      LOCAL_DATA, "field", "TEXT", "content", "TEXT", "last_time", "INTEGER",
                                      "exp_time", "INTEGER");
        if (!this->Exec(sql.c_str()))
        {
            return false;
        }
        return this->InvokeSqlFromPath(this->mConfig.script);
    }

    bool SqliteComponent::InvokeSqlFromPath(const std::string& path)
    {
        if (!this->mConfig.script.empty())
        {
            std::string sql;
            if (!help::fs::ReadTxtFile(this->mConfig.script, sql))
            {
                LOG_ERROR("load sqlite script fail")
                return false;
            }
            return this->Exec(sql.c_str());
        }

        return true;
    }

    bool SqliteComponent::OnHotFix()
    {
        return this->InvokeSqlFromPath(this->mConfig.script);
    }

    int SqliteComponent::MakeTable(const std::string& table,
                                   const pb::Message& message, const std::vector<std::string>& keys)
    {
        std::string sql;
        SqlHelper sqlHelper;
        if (!sqlHelper.Create(table, message, keys, sql))
        {
            return XCode::CreateSqlFail;
        }
        return this->Exec(sql.c_str());
    }


    void SqliteComponent::OnDestroy()
    {
        sqlite3_close(this->mDatabase);
    }

    bool SqliteComponent::Exec(const char* sql)
    {
        std::string err;
        if (!this->Exec(sql, err))
        {
            LOG_ERROR("err={}", err);
            LOG_ERROR("sql={}", sql);
            return false;
        }
        return true;
    }

    bool SqliteComponent::Exec(const char* sql, std::string& err)
    {
        char* errMessage = nullptr;
        if (sqlite3_exec(this->mDatabase, sql, nullptr, nullptr, &errMessage) != SQLITE_OK)
        {
            err = errMessage;
            sqlite3_free(errMessage);
            return false;
        }
        return true;
    }


    bool SqliteComponent::Query(const char* sql, std::vector<std::string>& result)
    {
        sqlite3_stmt* stmt;
        int code = sqlite3_prepare_v2(this->mDatabase, sql, -1, &stmt, nullptr);
        if (code != SQLITE_OK)
        {
            LOG_ERROR(sqlite3_errmsg(this->mDatabase));
            return false;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            json::w::Document document;
            int count = sqlite3_column_count(stmt);
            for (int index = 0; index < count; index++)
            {
                const char* key = sqlite3_column_name(stmt, index);
                switch (sqlite3_column_type(stmt, index))
                {
                case SQLITE_INTEGER:
                    {
                        long long value = sqlite3_column_int64(stmt, index);
                        document.Add(key, value);
                        break;
                    }
                case SQLITE_FLOAT:
                    {
                        double value = sqlite3_column_double(stmt, index);
                        document.Add(key, value);
                    }
                    break;
                case SQLITE_TEXT:
                    {
                        const unsigned char* value = sqlite3_column_text(stmt, index);
                        const int length = sqlite3_column_bytes(stmt, index);
                        document.Add(key, (const char*)value, length);
                        break;
                    }
                case SQLITE_BLOB:
                    {
                        const void* data = sqlite3_column_blob(stmt, index);
                        const int length = sqlite3_column_bytes(stmt, index);
                        document.AddObject(key, (const char*)data, length);
                        break;
                    }
                case SQLITE_NULL:
                    {
                        document.AddNull(key);
                        break;
                    }
                default:
                    LOG_ERROR("[{}] unknown field type", sqlite3_column_type(stmt, index));
                    sqlite3_finalize(stmt);
                    return false;
                }
            }
            std::string json;
            document.Encode(&json);
            result.emplace_back(json);
        }
        sqlite3_finalize(stmt);
        return true;
    }

    bool SqliteComponent::Del(const std::string& key)
    {
        std::string sql = fmt::format("DELETE FROM {} WHERE field={}", LOCAL_DATA, key);
        return this->Exec(sql.c_str());
    }

    bool SqliteComponent::Get(const std::string& key, std::string& value)
    {
        sqlite3_stmt* stmt;
        std::vector<std::string> results;
        std::string sql = fmt::format("SELECT content,exp_time FROM {} WHERE field='{}'", LOCAL_DATA, key);
        if (sqlite3_prepare_v2(this->mDatabase, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            value.append(sqlite3_errmsg(this->mDatabase));
            return false;
        }
        sqlite3_step(stmt);
        long long expTime = 0;
        int count = sqlite3_column_count(stmt);
        for (int index = 0; index < count; index++)
        {
            switch (sqlite3_column_type(stmt, index))
            {
            case SQLITE_INTEGER:
                {
                    expTime = sqlite3_column_int64(stmt, index);
                    break;
                }
            case SQLITE_TEXT:
                {
                    const unsigned char* data = sqlite3_column_text(stmt, index);
                    const int length = sqlite3_column_bytes(stmt, index);
                    value.append((const char*)data, length);
                    break;
                }
            }
        }
        sqlite3_finalize(stmt);
        if (expTime > 0)
        {
            long long nowTime = help::Time::NowSec();
            if (nowTime >= expTime)
            {
                value.clear();
                this->Del(key);
                return false;
            }
        }
        return true;
    }

    bool SqliteComponent::Set(const std::string& key, const std::string& value)
    {
        long long nowTime = help::Time::NowSec();
        std::string sql = fmt::format("REPLACE INTO {}(field,content,last_time)VALUES('{}','{}',{});",
                                      LOCAL_DATA, key, value, nowTime);
        return this->Exec(sql.c_str());
    }

    bool SqliteComponent::SetTimeout(const std::string& key, int timeout)
    {
        long long expTime = help::Time::NowSec() + timeout;
        std::string sql = fmt::format("UPDATE {} SET exp_time={} WHERE field='{}';", LOCAL_DATA, expTime, key);
        return this->Exec(sql.c_str());
    }
}
