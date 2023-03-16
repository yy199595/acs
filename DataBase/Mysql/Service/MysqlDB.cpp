#include"MysqlDB.h"
#include"Proto/ProtoHelper.h"
#include"Client/MysqlMessage.h"
#include"Component/MysqlDBComponent.h"
#include"Component/ProtoComponent.h"
namespace Sentry
{

    bool MysqlDB::Awake()
    {
        return this->mApp->AddComponent<MysqlDBComponent>();
    }

	bool MysqlDB::OnStart()
    {
        BIND_COMMON_RPC_METHOD(MysqlDB::Add);
        BIND_COMMON_RPC_METHOD(MysqlDB::Save);
        BIND_COMMON_RPC_METHOD(MysqlDB::Query);
        BIND_COMMON_RPC_METHOD(MysqlDB::Update);
        BIND_COMMON_RPC_METHOD(MysqlDB::Delete);
        BIND_COMMON_RPC_METHOD(MysqlDB::Create);
        this->mProtoComponent = this->GetComponent<ProtoComponent>();
        this->mMysqlComponent = this->GetComponent<MysqlDBComponent>();
        for (int index = 0; index < MysqlConfig::Inst()->MaxCount; index++)
        {
            this->mMysqlComponent->MakeMysqlClient();
        }
        return this->mMysqlComponent->Ping(0);
    }

    void MysqlDB::OnClose()
    {
        this->WaitAllMessageComplete();
    }

    int MysqlDB::Create(const db::mysql::create &request)
    {
        std::shared_ptr<Message> message = this->mProtoComponent->New(request.data());
        if (message == nullptr)
        {
            return XCode::CallArgsError;
        }
        const std::string typeName = message->GetTypeName();
        if(typeName.find('.') == std::string::npos)
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
            keys.emplace_back(std::move(key));
        }
        if(keys.size() == 1)
        {
            this->mMainKeys[typeName] = keys[0];
        }
        std::shared_ptr<Mysql::CreateTabCommand> command =
			std::make_shared<Mysql::CreateTabCommand>(message, keys);

        if (!this->mMysqlComponent->Run(command))
        {
            return XCode::Failure;
        }
        return XCode::Successful;
    }

	int MysqlDB::Add(const db::mysql::add& request)
    {
		if(!request.has_data())
		{
			return XCode::CallArgsError;
		}
		const Any & data = request.data();
		std::shared_ptr<Message> message = this->mProtoComponent->New(data);
		if(message == nullptr)
		{
			return XCode::CreateProtoFailure;
		}
		std::string sql;
		if(!this->mSqlHelper.Insert(*message, sql))
		{
			return XCode::Failure;
		}

		int index = request.flag();
		std::shared_ptr<Mysql::SqlCommand> command = std::make_shared<Mysql::SqlCommand>(sql);
		return this->mMysqlComponent->Execute(index, command) ? XCode::Successful : XCode::MysqlInvokeFailure;
    }

	int MysqlDB::Save(const db::mysql::save& request)
    {
		if(!request.has_data())
		{
			return XCode::CallArgsError;
		}
		const Any & data = request.data();
		std::shared_ptr<Message> message = this->mProtoComponent->New(data);
		if(message == nullptr)
		{
			return XCode::CreateProtoFailure;
		}
		std::string sql;
		if (!this->mSqlHelper.Replace(*message, sql))
        {
            return XCode::CallArgsError;
        }
		int index = request.flag();
		std::shared_ptr<Mysql::SqlCommand> command = std::make_shared<Mysql::SqlCommand>(sql);
		return this->mMysqlComponent->Execute(index, command) ? XCode::Successful : XCode::MysqlInvokeFailure;
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
		int index = request.flag();
		const std::string & fullName = request.table();
		std::shared_ptr<Mysql::SqlCommand> command = std::make_shared<Mysql::SqlCommand>(sql);
		return this->mMysqlComponent->Execute(index, command) ? XCode::Successful : XCode::MysqlInvokeFailure;
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
		int index = request.flag();
		std::shared_ptr<Mysql::SqlCommand> command = std::make_shared<Mysql::SqlCommand>(sql);
		std::shared_ptr<Mysql::Response> response = this->mMysqlComponent->Run(index, command);
		return this->mMysqlComponent->Execute(index, command)? XCode::Successful : XCode::MysqlInvokeFailure;
    }

	int MysqlDB::Query(const db::mysql::query& request, db::mysql::response& response)
    {
		std::shared_ptr<Mysql::QueryCommand> command;
		if(request.sql().empty())
		{
			std::string sql;
			std::vector<std::string> fields;
			const int limit = request.limit();
			const std::string& table = request.table();
			const std::string & where = request.where_json();
			if (!this->mSqlHelper.Select(table, where, fields, limit, sql))
			{
				return XCode::CallArgsError;
			}
			command = std::make_shared<Mysql::QueryCommand>(sql);
		}
		else
		{
			const std::string & sql = request.sql();
			command = std::make_shared<Mysql::QueryCommand>(sql);
		}
		std::shared_ptr<Mysql::Response> result = this->mMysqlComponent->Run(command);
		if(!result->IsOk())
		{
			response.set_error(result->GetError());
			return XCode::MysqlInvokeFailure;
		}
        for (size_t index = 0; index < result->size(); index++)
		{
			Json::Writer* document = result->at(index);
			document->WriterStream(response.add_jsons());
		}
        return XCode::Successful;
    }
}// namespace Sentry