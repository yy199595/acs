#include"MysqlService.h"
#include"Proto/ProtoHelper.h"
#include"Client/MysqlMessage.h"
#include"Component/MysqlDBComponent.h"
#include"Component/ProtoComponent.h"
#include"Component/DataSyncComponent.h"
namespace Sentry
{

	bool MysqlService::OnStartService()
	{
        BIND_COMMON_RPC_METHOD(MysqlService::Add);
        BIND_COMMON_RPC_METHOD(MysqlService::Save);
        BIND_COMMON_RPC_METHOD(MysqlService::Query);
        BIND_COMMON_RPC_METHOD(MysqlService::Update);
        BIND_COMMON_RPC_METHOD(MysqlService::Delete);
        BIND_COMMON_RPC_METHOD(MysqlService::Create);
        this->mSyncComponent = this->GetComponent<DataSyncComponent>();
        LOG_CHECK_RET_FALSE(this->mProtoComponent = this->GetComponent<ProtoComponent>());
        LOG_CHECK_RET_FALSE(this->mMysqlComponent = this->GetComponent<MysqlDBComponent>());
        this->mMysqlHelper = std::make_shared<MysqlHelper>(this->mProtoComponent);
        return true;
	}

    XCode MysqlService::Create(const db::mysql::create &request)
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

        std::shared_ptr<Mysql::CreateTabCommand> command
            = std::make_shared<Mysql::CreateTabCommand>(message, keys);
        std::shared_ptr<MysqlClient> mysqlClient =
            this->mMysqlComponent->GetClient(0);

        if (!this->mMysqlComponent->Run(mysqlClient, command))
        {
            return XCode::Failure;
        }
        return XCode::Successful;
    }

	XCode MysqlService::Add(const db::mysql::add& request)
    {
        std::string sql;
        std::shared_ptr<Message> message;
        if (!this->mMysqlHelper->ToSqlCommand(request, sql, message))
        {
            return XCode::CallArgsError;
        }
        std::shared_ptr<Mysql::SqlCommand> command
            = std::make_shared<Mysql::SqlCommand>(sql);
        std::shared_ptr<MysqlClient> mysqlClient =
            this->mMysqlComponent->GetClient(request.flag());

        if (!this->mMysqlComponent->Run(mysqlClient, command))
        {
            return XCode::Failure;
        }
        if (this->mSyncComponent != nullptr && message != nullptr)
        {
            std::string fullName = message->GetTypeName();
            auto iter = this->mMainKeys.find(fullName);
            if(iter != this->mMainKeys.end())
            {
                std::string key, value;
                const std::string & field = iter->second;
                if (Helper::Protocol::GetMember(field, *message, key))
                {
                    std::string json;
                    if (Helper::Protocol::GetJson(*message, &json))
                    {
                        this->mSyncComponent->Set(key, fullName, json);
                    }
                }
            }
        }
        return XCode::Successful;
    }

	XCode MysqlService::Save(const db::mysql::save& request)
    {
        std::string sql;
        std::shared_ptr<Message> message;
        if (!this->mMysqlHelper->ToSqlCommand(request, sql, message))
        {
            return XCode::CallArgsError;
        }
        std::shared_ptr<Mysql::SqlCommand> command
            = std::make_shared<Mysql::SqlCommand>(sql);
        std::shared_ptr<MysqlClient> mysqlClient =
            this->mMysqlComponent->GetClient(request.flag());

        if (this->mSyncComponent != nullptr && message != nullptr)
        {
            std::string fullName = message->GetTypeName();
            auto iter = this->mMainKeys.find(fullName);
            if(iter != this->mMainKeys.end())
            {
                std::string key;
                const std::string & field = iter->second;
                if (Helper::Protocol::GetMember(field, *message, key))
                {
                    this->mSyncComponent->Del(key, fullName);
                }
            }
        }
        if (!this->mMysqlComponent->Run(mysqlClient, command))
        {
            return XCode::Failure;
        }
        return XCode::Successful;
    }

	XCode MysqlService::Update(const db::mysql::update& request)
    {
        std::string sql, key, value;
        const std::string & fullName = request.table();
        auto iter = this->mMainKeys.find(fullName);
        if(iter != this->mMainKeys.end())
        {
            key = iter->second;
        }
        if (!this->mMysqlHelper->ToSqlCommand(request, sql, key, value))
        {
            return XCode::CallArgsError;
        }

        if(this->mSyncComponent != nullptr && !key.empty() && !value.empty())
        {
            this->mSyncComponent->Del(value, fullName);
        }
        std::shared_ptr<Mysql::SqlCommand> command
            = std::make_shared<Mysql::SqlCommand>(sql);

        std::shared_ptr<MysqlClient> mysqlClient =
            this->mMysqlComponent->GetClient(request.flag());
        if(!this->mMysqlComponent->Run(mysqlClient, command))
        {
            return XCode::Failure;
        }
        return XCode::Successful;
    }

	XCode MysqlService::Delete(const db::mysql::remove& request)
    {
        std::string sql, key, value;
        auto iter = this->mMainKeys.find(request.table());
        if(iter != this->mMainKeys.end())
        {
            key = iter->second;
        }
        if (!this->mMysqlHelper->ToSqlCommand(request, sql, key, value))
        {
            return XCode::CallArgsError;
        }
        if(this->mSyncComponent != nullptr && !value.empty())
        {
            this->mSyncComponent->Del(value, request.table());
        }
        std::shared_ptr<Mysql::SqlCommand> command
            = std::make_shared<Mysql::SqlCommand>(sql);
        std::shared_ptr<MysqlClient> mysqlClient =
            this->mMysqlComponent->GetClient(request.flag());

        if(!this->mMysqlComponent->Run(mysqlClient, command))
        {
            return XCode::Failure;
        }
        return XCode::Successful;
    }

	XCode MysqlService::Query(const db::mysql::query& request, db::mysql::response& response)
    {
        std::string sql, key, value;
        const std::string & fullName = request.table();
        auto iter = this->mMainKeys.find(fullName);
        if(iter != this->mMainKeys.end())
        {
            key = iter->second;
        }
        if (!this->mMysqlHelper->ToSqlCommand(request, sql, key, value))
        {
            return XCode::CallArgsError;
        }
        std::shared_ptr<MysqlClient> mysqlClient
            = this->mMysqlComponent->GetClient();

        std::shared_ptr<Mysql::QueryCommand> command
            = std::make_shared<Mysql::QueryCommand>(sql);

        if (!this->mMysqlComponent->Run(mysqlClient, command))
        {
            return XCode::Failure;
        }
        if(!value.empty() && this->mSyncComponent != nullptr && command->size() == 1)
        {
            rapidjson::Document * document = command->at(0);
            if(document != nullptr)
            {
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

                document->Accept(writer);
                const std::string json(buffer.GetString(), buffer.GetLength());
                this->mSyncComponent->Set(value, request.table(), json);
            }
        }

        for (size_t index = 0; index < command->size(); index++)
        {
            rapidjson::Document * document = command->at(index);
            if(document != nullptr)
            {
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                document->Accept(writer);

                response.add_jsons(buffer.GetString(), buffer.GetLength());
            }
        }
        return XCode::Successful;
    }
}// namespace Sentry