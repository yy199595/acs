#include"MysqlService.h"
#include"Proto/ProtoHelper.h"
#include"Client/MysqlMessage.h"
#include"Component/MysqlDBComponent.h"
#include"Component/ProtoComponent.h"
#ifdef __ENABLE_REDIS__
#include"Component/DataSyncComponent.h"
#endif
namespace Sentry
{

    bool MysqlService::Awake()
    {
        return this->mApp->AddComponent<MysqlDBComponent>();
    }

	bool MysqlService::OnStart()
	{
        BIND_COMMON_RPC_METHOD(MysqlService::Add);
        BIND_COMMON_RPC_METHOD(MysqlService::Save);
        BIND_COMMON_RPC_METHOD(MysqlService::Query);
        BIND_COMMON_RPC_METHOD(MysqlService::Update);
        BIND_COMMON_RPC_METHOD(MysqlService::Delete);
        BIND_COMMON_RPC_METHOD(MysqlService::Create);
        this->mMysqlComponent = this->GetComponent<MysqlDBComponent>();
#ifdef __ENABLE_REDIS__
        this->mSyncComponent = this->GetComponent<DataSyncComponent>();
#endif
        LOG_CHECK_RET_FALSE(this->mProtoComponent = this->GetComponent<ProtoComponent>());
        this->mMysqlHelper = std::make_shared<MysqlHelper>(this->mProtoComponent);
        return this->mMysqlComponent->StartConnectMysql();
	}

    bool MysqlService::OnClose()
    {
        this->WaitAllMessageComplete();
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

		MysqlClient * mysqlClient = this->mMysqlComponent->GetClient(0);
        std::shared_ptr<Mysql::CreateTabCommand> command = std::make_shared<Mysql::CreateTabCommand>(message, keys);

        if (!this->mMysqlComponent->Run(mysqlClient, command))
        {
            return XCode::Failure;
        }
        return XCode::Successful;
    }

	XCode MysqlService::Add(const db::mysql::add& request)
    {
        std::string sql;
        if (!this->mMysqlHelper->ToSqlCommand(request, sql))
        {
            return XCode::CallArgsError;
        }
        std::shared_ptr<Message> message = this->mMysqlHelper->GetData();
        MysqlClient * mysqlClient = this->mMysqlComponent->GetClient(request.flag());
		std::shared_ptr<Mysql::SqlCommand> command = std::make_shared<Mysql::SqlCommand>(sql);

		if (!this->mMysqlComponent->Run(mysqlClient, command))
        {
            return XCode::Failure;
        }
#ifdef __ENABLE_REDIS__
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
#endif
        return XCode::Successful;
    }

	XCode MysqlService::Save(const db::mysql::save& request)
    {
        std::string sql;
        if (!this->mMysqlHelper->ToSqlCommand(request, sql))
        {
            return XCode::CallArgsError;
        }
		MysqlClient * mysqlClient = this->mMysqlComponent->GetClient(request.flag());
		std::shared_ptr<Mysql::SqlCommand> command = std::make_shared<Mysql::SqlCommand>(sql);
#ifdef __ENABLE_REDIS__
        std::shared_ptr<Message> message = this->mMysqlHelper->GetData();
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
#endif
        if (!this->mMysqlComponent->Run(mysqlClient, command))
        {
            return XCode::Failure;
        }
        return XCode::Successful;
    }

	XCode MysqlService::Update(const db::mysql::update& request)
    {
        std::string sql;
        if (!this->mMysqlHelper->ToSqlCommand(request, sql))
        {
            return XCode::CallArgsError;
        }
        const std::string & fullName = request.table();
#ifdef __ENABLE_REDIS__
        auto iter = this->mMainKeys.find(fullName);
        if(this->mSyncComponent != nullptr && iter != this->mMainKeys.end())
        {
            std::string value;
            if (this->mMysqlHelper->GetValue(iter->second, value))
            {
                this->mSyncComponent->Del(value, fullName);
            }
        }
#endif
        MysqlClient * mysqlClient = this->mMysqlComponent->GetClient(request.flag());
		std::shared_ptr<Mysql::SqlCommand> command = std::make_shared<Mysql::SqlCommand>(sql);

        if(!this->mMysqlComponent->Run(mysqlClient, command))
        {
            return XCode::Failure;
        }
        return XCode::Successful;
    }

	XCode MysqlService::Delete(const db::mysql::remove& request)
    {
        std::string sql;
        if (!this->mMysqlHelper->ToSqlCommand(request, sql))
        {
            return XCode::CallArgsError;
        }
#ifdef __ENABLE_REDIS__
        auto iter = this->mMainKeys.find(request.table());
        if(this->mSyncComponent != nullptr && iter != this->mMainKeys.end())
        {
            std::string value;
            if (this->mMysqlHelper->GetValue(iter->second, value))
            {
                this->mSyncComponent->Del(value, request.table());
            }
        }
#endif

        MysqlClient * mysqlClient = this->mMysqlComponent->GetClient(request.flag());
		std::shared_ptr<Mysql::SqlCommand> command = std::make_shared<Mysql::SqlCommand>(sql);

        if(!this->mMysqlComponent->Run(mysqlClient, command))
        {
            return XCode::Failure;
        }
        return XCode::Successful;
    }

	XCode MysqlService::Query(const db::mysql::query& request, db::mysql::response& response)
    {
		std::shared_ptr<Mysql::QueryCommand> command;
		const std::string & fullName = request.table();
		if(request.sql().empty())
		{
			std::string sql;
			const std::string& fullName = request.table();
			if (!this->mMysqlHelper->ToSqlCommand(request, sql))
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
		MysqlClient * mysqlClient = this->mMysqlComponent->GetClient();
		if (!this->mMysqlComponent->Run(mysqlClient, command))
        {
            return XCode::Failure;
        }

        if(command->size() == 0)
        {
            throw std::logic_error("query data form " + fullName + " size = 0");
        }

        auto iter = this->mMainKeys.find(fullName);
        for (size_t index = 0; index < command->size(); index++)
        {
            Json::Document * document = command->at(index);
            if(document != nullptr)
            {
                rapidjson::Document & doc = (rapidjson::Document&)(*document);
                std::string * json = document->Serialize(response.add_jsons());
#ifdef __ENABLE_REDIS__
                if (request.fields_size() == 0 && this->mSyncComponent != nullptr && iter != this->mMainKeys.end())
                {
                    std::string value;
                    const std::string &field = iter->second;
                    if (this->mMysqlHelper->GetValue(doc, field, value))
                    {
                        this->mSyncComponent->Set(value, fullName, *json);
                    }
                }
#endif
            }
        }

        return XCode::Successful;
    }
}// namespace Sentry