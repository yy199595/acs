
#ifdef __ENABLE_MYSQL__

#include"MysqlDB.h"
#include"Entity/Actor/App.h"
#include"Util/Proto/ProtoHelper.h"
#include"Mysql/Client/MysqlMessage.h"
#include"Proto/Component/ProtoComponent.h"
#include"Mysql/Component/MysqlDBComponent.h"

namespace Tendo
{
	MysqlDB::MysqlDB()
	{
		this->mMysqlComponent = nullptr;
		this->mProtoComponent = nullptr;
	}

	bool MysqlDB::OnInit()
	{
		BIND_COMMON_RPC_METHOD(MysqlDB::Add);
		BIND_COMMON_RPC_METHOD(MysqlDB::Save);
		BIND_COMMON_RPC_METHOD(MysqlDB::Exec);
		BIND_COMMON_RPC_METHOD(MysqlDB::Query);
		BIND_COMMON_RPC_METHOD(MysqlDB::Update);
		BIND_COMMON_RPC_METHOD(MysqlDB::Delete);
		BIND_COMMON_RPC_METHOD(MysqlDB::Create);
		this->mApp->AddComponent<MysqlDBComponent>();
		return true;
	}

	void MysqlDB::OnStart()
    {
        this->mProtoComponent = this->GetComponent<ProtoComponent>();
        this->mMysqlComponent = this->GetComponent<MysqlDBComponent>();
		const MysqlConfig & config = this->mMysqlComponent->Config();
        for (int index = 0; index <config.MaxCount; index++)
        {
			int id = 0;
			if(this->mMysqlComponent->GetClientHandle(id))
			{
				this->mClientIdQueue.emplace(id);
				this->mClientIds.emplace_back(id);
			}
        }
        this->mMysqlComponent->Ping(0);
    }

    void MysqlDB::OnStop()
    {
        	
    }

    int MysqlDB::Create(const db::mysql::create &request)
    {
        std::shared_ptr<Message> message;
		if(!this->mProtoComponent->New(request.data(), message))
		{
			return XCode::CallArgsError;
		}
		const std::string & table = request.table();
        if(table.find('.') == std::string::npos)
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

        std::shared_ptr<Mysql::CreateTabCommand> command =
			std::make_shared<Mysql::CreateTabCommand>(table, message, keys);
		int id = this->mClientIds.front();
        if (!this->mMysqlComponent->Run(id, command))
        {
            return XCode::Failure;
        }
        return XCode::Successful;
    }

	int MysqlDB::GetClientId(int flag)
	{
		if(flag > 0)
		{
			int index = flag % (int)this->mClientIds.size();
			return this->mClientIds.at(index);
		}
		int id = this->mClientIdQueue.front();
		this->mClientIdQueue.push(id);
		return id;
	}

	int MysqlDB::Add(const db::mysql::add& request)
    {
		if(!request.has_data())
		{
			return XCode::CallArgsError;
		}
		const Any & data = request.data();
		std::shared_ptr<Message> message;
		if(!this->mProtoComponent->New(data, message))
		{
			return XCode::CreateProtoFailure;
		}
		std::string sql;
		if(!this->mSqlHelper.Insert(*message, sql))
		{
			return XCode::Failure;
		}
		int id = this->GetClientId(request.flag());
		std::shared_ptr<Mysql::SqlCommand> command = std::make_shared<Mysql::SqlCommand>(sql);
		return this->mMysqlComponent->Execute(id, command) ? XCode::Successful : XCode::MysqlInvokeFailure;
    }

	int MysqlDB::Save(const db::mysql::save& request)
    {
		if(!request.has_data())
		{
			return XCode::CallArgsError;
		}
		std::shared_ptr<Message> message;
		const Any & data = request.data();
		if(!this->mProtoComponent->New(data, message))
		{
			return XCode::CreateProtoFailure;
		}
		std::string sql;
		if (!this->mSqlHelper.Replace(*message, sql))
        {
            return XCode::CallArgsError;
        }
		int id = this->GetClientId(request.flag());
		std::shared_ptr<Mysql::SqlCommand> command = std::make_shared<Mysql::SqlCommand>(sql);
		return this->mMysqlComponent->Execute(id, command) ? XCode::Successful : XCode::MysqlInvokeFailure;
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
		int id = this->GetClientId(request.flag());
		const std::string & fullName = request.table();
		std::shared_ptr<Mysql::SqlCommand> command = std::make_shared<Mysql::SqlCommand>(sql);
		return this->mMysqlComponent->Execute(id, command) ? XCode::Successful : XCode::MysqlInvokeFailure;
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
		int id = this->GetClientId(request.flag());
		std::shared_ptr<Mysql::SqlCommand> command = std::make_shared<Mysql::SqlCommand>(sql);
		return this->mMysqlComponent->Execute(id, command)? XCode::Successful : XCode::MysqlInvokeFailure;
    }

	int MysqlDB::Exec(const db::mysql::exec& request, db::mysql::response& response)
	{
		int id = this->GetClientId();
		const std::string & sql = request.sql();
		if(!request.query())
		{
			std::shared_ptr<Mysql::ICommand> command = std::make_shared<Mysql::SqlCommand>(sql);
			std::shared_ptr<Mysql::Response> result = this->mMysqlComponent->Run(id, command);
			if (!result->IsOk())
			{
				response.set_error(result->GetError());
				return XCode::MysqlInvokeFailure;
			}
			return XCode::Successful;
		}
		std::shared_ptr<Mysql::ICommand> command = std::make_shared<Mysql::QueryCommand>(sql);
		std::shared_ptr<Mysql::Response> result = this->mMysqlComponent->Run(id, command);
		if (!result->IsOk())
		{
			response.set_error(result->GetError());
			return XCode::MysqlInvokeFailure;
		}
		for (size_t index = 0; index < result->ArraySize(); index++)
		{
			response.add_jsons(result->Get(index));
		}
		return XCode::Successful;
	}

	int MysqlDB::Query(const db::mysql::query& request, db::mysql::response& response)
	{
		std::string sql;
		std::vector<std::string> fields;
		const int limit = request.limit();
		const std::string& table = request.table();
		const std::string& where = request.where_json();
		if (!this->mSqlHelper.Select(table, where, fields, limit, sql))
		{
			return XCode::CallArgsError;
		}
		int id = this->GetClientId();
		std::shared_ptr<Mysql::QueryCommand> command = std::make_shared<Mysql::QueryCommand>(sql);
		std::shared_ptr<Mysql::Response> result = this->mMysqlComponent->Run(id, command);
		if (!result->IsOk())
		{
			response.set_error(result->GetError());
			return XCode::MysqlInvokeFailure;
		}
		for (size_t index = 0; index < result->ArraySize(); index++)
		{
			response.add_jsons(result->Get(index));
		}
		return XCode::Successful;
	}
}// namespace Sentry

#endif