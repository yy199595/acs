#include"SqliteComponent.h"
#include"Config/ServerConfig.h"
#include"File/DirectoryHelper.h"
#include "Json/JsonWriter.h"
#include"String/StringHelper.h"
#include"Helper/SqlHelper.h"
namespace Sentry
{
	bool SqliteComponent::LateAwake()
	{
		if (!ServerConfig::Inst()->GetPath("sqlite", this->mPath))
		{
			return false;
		}
		Helper::Directory::MakeDir(this->mPath);
		return true;
	}

	bool SqliteComponent::MakeTable(int id, const std::string & key, const google::protobuf::Message& message)
	{
		std::vector<std::string> result;
		const std::string name = message.GetTypeName();
		if(!Helper::Str::Split(name, ".", result))
		{
			return false;
		}
		std::string sql;
		SqlHelper sqlHelper;
		const std::string & tab = result[1];
		if(!sqlHelper.Create(message, tab, {key}, sql))
		{
			return false;
		}
		return this->Exec(id, sql);
	}

	int SqliteComponent::Open(const std::string& name)
	{
		sqlite3 * db = nullptr;
		std::string path = fmt::format("{0}/{1}.db", this->mPath, name);
		if(sqlite3_open(path.c_str(), &db) != SQLITE_OK)
		{
			return false;
		}
		int id = this->mNumbers.Pop();
		this->mDatabases.emplace(id, db);
		return id;
	}

	void SqliteComponent::Close(int id)
	{
		auto iter = this->mDatabases.find(id);
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

	bool SqliteComponent::Exec(int id, const std::string& sql)
	{
		auto iter = this->mDatabases.find(id);
		if(iter == this->mDatabases.end())
		{
			return false;
		}
		char* errMessage = 0;
		sqlite3 * db = iter->second;
		if (sqlite3_exec(db, sql.c_str(), 0, 0, &errMessage) != SQLITE_OK)
		{
			LOG_FATAL(sql);
			LOG_ERROR(errMessage);
			sqlite3_free(errMessage);
			return false;
		}
		return true;
	}

	bool SqliteComponent::Query(int id, const std::string& sql, std::vector<std::string>& result)
	{
		auto iter = this->mDatabases.find(id);
		if (iter == this->mDatabases.end())
		{
			return false;
		}
		sqlite3_stmt* stmt;
		sqlite3* db = iter->second;
		int code = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
		if (code != SQLITE_OK)
		{
			LOG_ERROR(sqlite3_errmsg(db));
			return false;
		}
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			Json::Writer document;
			int count = sqlite3_column_count(stmt);
			for (int index = 0; index < count; index++)
			{
				std::string key(sqlite3_column_name(stmt, index));
				switch (sqlite3_column_type(stmt, index))
				{
					case SQLITE_INTEGER:
					{
						long long value = sqlite3_column_int64(stmt, index);
						document.Add(key).Add(value);
					}
						break;
					case SQLITE_FLOAT:
					{
						double value = sqlite3_column_double(stmt, index);
						document.Add(key).Add(value);
					}
						break;
					case SQLITE_TEXT:
					{
						const unsigned char* value = sqlite3_column_text(stmt, index);
						document.Add(key).Add(std::string((const char*)value));
					}
						break;
					default:
						sqlite3_finalize(stmt);
						LOG_ERROR("unknown field type");
						return false;
				}
			}
			std::string json;
			document.WriterStream(&json);
			result.emplace_back(json);
		}
		sqlite3_finalize(stmt);
		return true;
	}


}