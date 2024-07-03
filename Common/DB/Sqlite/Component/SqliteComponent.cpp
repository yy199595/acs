#include"SqliteComponent.h"
#include "XCode/XCode.h"
#include"Server/Config/ServerConfig.h"
#include"Util/File/DirectoryHelper.h"
#include"Util/String/String.h"
#include"Util/Sql/SqlHelper.h"
#include"Sqlite/Lua/LuaSqlite.h"
#include"Lua/Engine/ModuleClass.h"
namespace joke
{
	bool SqliteComponent::Awake()
	{
		if (!ServerConfig::Inst()->GetPath("sqlite", this->mSaveDir))
		{
			return false;
		}
		return help::dir::MakeDir(this->mSaveDir);
	}

	void SqliteComponent::OnLuaRegister(Lua::ModuleClass& luaRegister)
	{
		luaRegister.AddFunction("Exec", Lua::Sqlite::Exec);
		luaRegister.AddFunction("Find", Lua::Sqlite::Find);
		luaRegister.AddFunction("FindOne", Lua::Sqlite::FindOne);
		luaRegister.End("db.sqlite");
	}

	bool SqliteComponent::Splite(const std::string& table, std::string& db, std::string& tab)
	{
		size_t pos = table.find('.');
		if(pos == std::string::npos)
		{
			return false;
		}
		db = table.substr(0, pos);
		tab = table.substr(pos + 1);
		return true;
	}

	int SqliteComponent::MakeTable(const std::string & table,
			const pb::Message& message, const std::vector<std::string> & keys)
	{
		std::string db, tab;
		if(!SqliteComponent::Splite(table, db, tab))
		{
			return XCode::CallArgsError;
		}

		std::string sql;
		SqlHelper sqlHelper;
		if(!sqlHelper.Create(tab, message, keys, sql))
		{
			return XCode::CreateSqlFail;
		}
		return this->Exec(db, sql.c_str());
	}

	sqlite3 * SqliteComponent::GetClient(const std::string& db)
	{
		auto iter = this->mDatabases.find(db);
		if(iter != this->mDatabases.end())
		{
			return iter->second;
		}
		sqlite3 * sqlite = nullptr;
		std::string path = fmt::format("{0}/{1}.db", this->mSaveDir, db);
		if(sqlite3_open(path.c_str(), &sqlite) != SQLITE_OK)
		{
			LOG_ERROR("open sqlite db {} fail", db);
			return nullptr;
		}
		this->mDatabases.emplace(db, sqlite);
		return sqlite;
	}

	void SqliteComponent::Close(const std::string & db)
	{
		auto iter = this->mDatabases.find(db);
		if(iter != this->mDatabases.end())
		{
			sqlite3_close(iter->second);
			this->mDatabases.erase(iter);
		}
	}

	void SqliteComponent::OnDestroy()
	{
		auto iter = this->mDatabases.begin();
		for(; iter != this->mDatabases.end(); iter++)
		{
			sqlite3_close(iter->second);
		}
		this->mDatabases.clear();
	}

	int SqliteComponent::Exec(const std::string & db, const char* sql)
	{
		std::string err;
		int code = this->Exec(db, sql, err);
		if(code != XCode::Ok)
		{
			LOG_ERROR("err={}", err);
			LOG_ERROR("sql={}", sql);
			return code;
		}
		return code;

	}

	int SqliteComponent::Exec(const std::string & db, const char* sql, std::string& err)
	{
		sqlite3 * sqlite = this->GetClient(db);
		if(sqlite == nullptr)
		{
			err = fmt::format("open {} fail", db);
			return XCode::OpenDBFail;
		}
		char* errMessage = nullptr;
		if (sqlite3_exec(sqlite, sql, nullptr, nullptr, &errMessage) != SQLITE_OK)
		{
			err = errMessage;
			sqlite3_free(errMessage);
			return XCode::SqliteExecSqlFail;
		}
		return XCode::Ok;
	}


	int SqliteComponent::Query(const std::string & db, const char * sql, std::vector<std::string>& result)
	{
		sqlite3 * sqlite = this->GetClient(db);
		if(sqlite == nullptr)
		{
			return XCode::OpenDBFail;
		}

		sqlite3_stmt* stmt;
		int code = sqlite3_prepare_v2(sqlite, sql, -1, &stmt, nullptr);
		if (code != SQLITE_OK)
		{
			LOG_ERROR(sqlite3_errmsg(sqlite));
			return false;
		}
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			json::w::Document document;
			int count = sqlite3_column_count(stmt);
			for (int index = 0; index < count; index++)
			{
				const char * key = sqlite3_column_name(stmt, index);
				switch (sqlite3_column_type(stmt, index))
				{
					case SQLITE_INTEGER:
					{
						long long value = sqlite3_column_int64(stmt, index);
						document.Add(key, value);
					}
						break;
					case SQLITE_FLOAT:
					{
						double value = sqlite3_column_double(stmt, index);
						document.Add(key, value);
					}
						break;
					case SQLITE_TEXT:
					{
						const unsigned char* value = sqlite3_column_text(stmt, index);
						document.Add(key, std::string((const char*)value));
					}
						break;
					default:
						sqlite3_finalize(stmt);
						LOG_ERROR("unknown field type");
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


}