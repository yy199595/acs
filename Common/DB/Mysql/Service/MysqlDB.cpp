
#ifdef __ENABLE_MYSQL__

#include"MysqlDB.h"
#include"Entity/Actor/App.h"
#include"Mysql/Client/MysqlMessage.h"
#include"Proto/Component/ProtoComponent.h"
#include"Mysql/Component/MysqlDBComponent.h"

namespace acs
{
	MysqlDB::MysqlDB()
	{
		this->mMysqlComponent = nullptr;
		this->mProtoComponent = nullptr;
	}

	bool MysqlDB::Awake()
	{
		this->mApp->AddComponent<MysqlDBComponent>();
		return true;
	}

	bool MysqlDB::OnInit()
	{
		BIND_SERVER_RPC_METHOD(MysqlDB::Add);
		BIND_SERVER_RPC_METHOD(MysqlDB::Save);
		BIND_SERVER_RPC_METHOD(MysqlDB::Exec);
		BIND_SERVER_RPC_METHOD(MysqlDB::Query);
		BIND_SERVER_RPC_METHOD(MysqlDB::Update);
		BIND_SERVER_RPC_METHOD(MysqlDB::Delete);
		BIND_SERVER_RPC_METHOD(MysqlDB::Create);
		this->mProtoComponent = this->GetComponent<ProtoComponent>();
		this->mMysqlComponent = this->GetComponent<MysqlDBComponent>();
		return true;
	}

    void MysqlDB::OnStop()
    {
        	
    }

    int MysqlDB::Create(const db::mysql::create &request)
    {
        std::unique_ptr<Message> message;
		const std::string & table = request.table();
		if(!this->mProtoComponent->New(table, message))
		{
			return XCode::CallArgsError;
		}
        std::vector<std::string> keys;
        const Descriptor * descriptor = message->GetDescriptor();
        for (const std::string &key: request.keys())
        {
            const FieldDescriptor * fieldDescriptor = descriptor->FindFieldByName(key);
            if(fieldDescriptor == nullptr)
            {
                return XCode::CallArgsError;
            }
            switch(fieldDescriptor->type())
            {
                case FieldDescriptor::Type::TYPE_INT32:
                case FieldDescriptor::Type::TYPE_INT64:
                case FieldDescriptor::Type::TYPE_UINT32:
                case FieldDescriptor::Type::TYPE_UINT64:
                case FieldDescriptor::Type::TYPE_STRING:
                    break;
                default:
                    return XCode::CallArgsError;
            }
            keys.emplace_back(key);
        }
        if(keys.size() == 1)
        {
            this->mMainKeys[table] = keys[0];
        }

        std::unique_ptr<Mysql::MakeTabRequest> command =
			std::make_unique<Mysql::MakeTabRequest>(table, std::move(message), keys);
		return this->mMysqlComponent->Execute(std::move(command)) ? XCode::Ok : XCode::Failure;
    }

	int MysqlDB::Add(const db::mysql::add& request)
    {
		json::r::Document document;
		const std::string & tab = request.table();
		const std::string & json = request.data();
		if(!document.Decode(json.c_str(), json.size()))
		{
			return XCode::ParseJsonFailure;
		}

		std::string sql;
		if(!this->mSqlHelper.Insert(tab, document, sql))
		{
			return XCode::Failure;
		}
		std::unique_ptr<Mysql::SqlRequest> command = std::make_unique<Mysql::SqlRequest>(sql);
		return this->mMysqlComponent->Execute(std::move(command))? XCode::Ok : XCode::MysqlInvokeFailure;
    }

	int MysqlDB::Save(const db::mysql::save& request)
    {
		json::r::Document document;
		const std::string & tab = request.table();
		const std::string & json = request.data();
		if(!document.Decode(json.c_str(), json.size()))
		{
			return XCode::ParseJsonFailure;
		}
		std::string sql;
		if (!this->mSqlHelper.Replace(tab, document, sql))
        {
            return XCode::CallArgsError;
        }
		std::unique_ptr<Mysql::SqlRequest> command = std::make_unique<Mysql::SqlRequest>(sql);
		return this->mMysqlComponent->Execute(std::move(command)) ? XCode::Ok : XCode::MysqlInvokeFailure;
    }

	int MysqlDB::Update(const db::mysql::update& request)
    {
        std::string sql;
		const std::string & table = request.table();
		const std::string & where = request.where_json();
		const std::string & update = request.update_json();
        if (!this->mSqlHelper.Update(table, where, update, sql))
        {
            return XCode::CallArgsError;
        }
		const std::string & fullName = request.table();
		std::unique_ptr<Mysql::SqlRequest> command = std::make_unique<Mysql::SqlRequest>(sql);
		return this->mMysqlComponent->Execute(std::move(command)) ? XCode::Ok : XCode::MysqlInvokeFailure;
    }

	int MysqlDB::Delete(const db::mysql::remove& request)
    {
        std::string sql;
		const std::string & table = request.table();
		const std::string & where = request.where_json();
		if (!this->mSqlHelper.Delete(table, where, sql))
        {
            return XCode::CallArgsError;
        }
		std::unique_ptr<Mysql::SqlRequest> command = std::make_unique<Mysql::SqlRequest>(sql);
		return this->mMysqlComponent->Execute(std::move(command)) ? XCode::Ok : XCode::MysqlInvokeFailure;
    }

	int MysqlDB::Exec(const db::mysql::exec& request, db::mysql::response& response)
	{
		const std::string & sql = request.sql();
		if(!request.query())
		{
			auto command = std::make_unique<Mysql::SqlRequest>(sql);
			Mysql::Response * result = this->mMysqlComponent->Run(std::move(command));
			if (!result->IsOk())
			{
				response.set_error(result->GetError());
				return XCode::MysqlInvokeFailure;
			}
			return XCode::Ok;
		}
		auto command = std::make_unique<Mysql::FindRequest>(sql);
		Mysql::Response * result = this->mMysqlComponent->Run(std::move(command));
		if (!result->IsOk())
		{
			response.set_error(result->GetError());
			return XCode::MysqlInvokeFailure;
		}
		for (size_t index = 0; index < result->ArraySize(); index++)
		{
			response.add_jsons(result->Get(index));
		}
		return XCode::Ok;
	}

	int MysqlDB::Query(const db::mysql::query& request, db::mysql::response& response)
	{
		const std::string& table = request.table();
		const std::string& where = request.where_json();

		LOG_ERROR_CHECK_ARGS(!table.empty());

		std::string sql;
		int limit = request.limit();
		std::vector<std::string> fields;
		if(request.fields_size() > 0)
		{
			fields.reserve(request.fields_size());
			for(int index = 0; index < request.fields_size(); index++)
			{
				fields.emplace_back(request.fields(index));
			}
		}
		if (!this->mSqlHelper.Select(table, where, fields, limit, sql))
		{
			return XCode::CallArgsError;
		}
		auto command = std::make_unique<Mysql::FindRequest>(sql);
		Mysql::Response * result = this->mMysqlComponent->Run(std::move(command));
		if (!result->IsOk())
		{
			response.set_error(result->GetError());
			return XCode::MysqlInvokeFailure;
		}
		for (size_t index = 0; index < result->ArraySize(); index++)
		{
			response.add_jsons(result->Get(index));
		}
		return XCode::Ok;
	}
}// namespace Sentry

#endif