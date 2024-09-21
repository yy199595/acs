//
// Created by leyi on 2023/7/25.
//

#include "SqliteDB.h"
#include "Entity/Actor/App.h"
#include "Util/Tools/String.h"
#include "Proto/Component/ProtoComponent.h"
#include "Sqlite/Component/SqliteComponent.h"

namespace acs
{
	bool SqliteDB::Awake()
	{
		this->mApp->AddComponent<SqliteComponent>();
		return true;
	}

	bool SqliteDB::OnInit()
	{
		this->mProto = App::GetProto();
		BIND_SERVER_RPC_METHOD(SqliteDB::Add);
		BIND_SERVER_RPC_METHOD(SqliteDB::Del);
		BIND_SERVER_RPC_METHOD(SqliteDB::Save);
		BIND_SERVER_RPC_METHOD(SqliteDB::Find);
		BIND_SERVER_RPC_METHOD(SqliteDB::Create);
		BIND_SERVER_RPC_METHOD(SqliteDB::Update);
		BIND_SERVER_RPC_METHOD(SqliteDB::FindOne);
		LOG_CHECK_RET_FALSE(this->mComponent = this->GetComponent<SqliteComponent>())
		return true;
	}

	int SqliteDB::Create(const db::mysql::create& request)
	{
		std::unique_ptr<pb::Message> data;
		if(!this->mProto->New(request.table(), data))
		{
			return XCode::CallArgsError;
		}
		std::string db, tab;
		const std::string& table = request.table();
		if(!help::Str::Split(table, '.', db, tab))
		{
			return XCode::CallArgsError;
		}
		std::string sql;
		std::vector<std::string> keys;
		keys.reserve(request.keys_size());
		for(int index = 0; index < request.keys_size(); index++)
		{
			keys.emplace_back(request.keys(index));
		}
		if(!this->mHelper.Create(tab, *data, keys, sql))
		{
			return XCode::CallArgsError;
		}
		return this->mComponent->Exec(db, sql.c_str());
	}

	int SqliteDB::Add(const db::mysql::add& request)
	{
		std::unique_ptr<pb::Message> data;
		if(!this->mProto->New(request.data(), data))
		{
			return XCode::CallArgsError;
		}
		std::string db, tab, sql;
		const std::string & json = request.data();
		const std::string & table = request.table();
		if(!help::Str::Split(table, '.', db, tab))
		{
			return XCode::CallArgsError;
		}
		json::r::Document document;
		if(!document.Decode(json.c_str(), json.size()))
		{
			return XCode::ParseJsonFailure;
		}
		if(!this->mHelper.Insert(tab, document, sql))
		{
			return XCode::CallArgsError;
		}
		return this->mComponent->Exec(db, sql.c_str());
	}

	int SqliteDB::Del(const db::mysql::remove& request)
	{
		std::string sql, db, tab;
		const std::string & table = request.table();
		if(!help::Str::Split(table, '.', db, tab))
		{
			return XCode::CallArgsError;
		}
		const std::string & where = request.where_json();
		if(!this->mHelper.Delete(tab, where, sql))
		{
			return XCode::CallArgsError;
		}
		return this->mComponent->Exec(db, sql.c_str());
	}

	int SqliteDB::Save(const db::mysql::save& request)
	{
		std::unique_ptr<pb::Message> data;
		if(!this->mProto->New(request.data(), data))
		{
			return XCode::CallArgsError;
		}
		std::string sql, db, tab;
		const std::string & json = request.data();
		const std::string & table = request.table();
		if(!help::Str::Split(table, '.', db, tab))
		{
			return XCode::CallArgsError;
		}
		json::r::Document document;
		if(!document.Decode(json.c_str(), json.size()))
		{
			return XCode::ParseJsonFailure;
		}
		if(!this->mHelper.Replace(tab, document, sql))
		{
			return XCode::CallArgsError;
		}
		return this->mComponent->Exec(db, sql.c_str());
	}

	int SqliteDB::Update(const db::mysql::update& request)
	{
		std::string sql, db, tab;
		const std::string & table = request.table();
		if(!help::Str::Split(table, '.', db, tab))
		{
			return XCode::CallArgsError;
		}
		const std::string & where = request.where_json();
		const std::string & update = request.update_json();
		if(!this->mHelper.Update(tab, where, update, sql))
		{
			return XCode::CallArgsError;
		}
		return this->mComponent->Exec(db, sql.c_str());
	}

	int SqliteDB::Find(const db::mysql::query& request, db::mysql::response& response)
	{
		std::string sql, db, tab;
		const std::string & table = request.table();
		const std::string & where = request.where_json();

		if(!help::Str::Split(table, '.', db, tab))
		{
			return XCode::CallArgsError;
		}
		std::vector<std::string> fields;
		fields.reserve(request.fields_size());
		for(int index = 0; index < request.fields_size(); index++)
		{
			fields.emplace_back(request.fields(index));
		}
		int limit = request.limit();
		if(!this->mHelper.Select(tab, where, fields, limit, sql))
		{
			return XCode::CallArgsError;
		}
		std::vector<std::string> result;
		int code = this->mComponent->Query(db, sql.c_str(), result);
		if(code != XCode::Ok)
		{
			return code;
		}
		for(const std::string & data : result)
		{
			response.add_jsons(data);
		}
		return XCode::Ok;
	}

	int SqliteDB::FindOne(const db::mysql::query& request, db::mysql::response& response)
	{
		std::string sql, db, tab;
		const std::string & table = request.table();
		const std::string & where = request.where_json();
		if(!help::Str::Split(table, '.', db, tab))
		{
			return XCode::CallArgsError;
		}
		std::vector<std::string> fields;
		fields.reserve(request.fields_size());
		for(int index = 0; index < request.fields_size(); index++)
		{
			fields.emplace_back(request.fields(index));
		}
		if(!this->mHelper.Select(tab, where, fields, 1, sql))
		{
			return XCode::CallArgsError;
		}
		std::vector<std::string> result;
		int code = this->mComponent->Query(db, sql.c_str(), result);
		if(code != XCode::Ok)
		{
			return code;
		}
		for(const std::string & data : result)
		{
			response.add_jsons(data);
			return XCode::Ok;
		}
		return code;
	}
}