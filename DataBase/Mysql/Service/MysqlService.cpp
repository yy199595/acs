#include"MysqlService.h"
#include"Client/MysqlMessage.h"
#include"Component/MysqlDBComponent.h"
#include"Component/Scene/ProtoComponent.h"
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
        if (message == nullptr || request.keys().empty())
        {
            return XCode::CallArgsError;
        }
        std::vector<std::string> keys;
        for (const std::string &key: request.keys())
        {
            keys.emplace_back(std::move(key));
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
        if (!this->mMysqlHelper->ToSqlCommand(request, sql))
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
        return XCode::Successful;
    }

	XCode MysqlService::Save(const db::mysql::save& request)
    {
        std::string sql;
        if (!this->mMysqlHelper->ToSqlCommand(request, sql))
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
        return XCode::Successful;
    }

	XCode MysqlService::Update(const db::mysql::update& request)
    {
        std::string sql;
        if (!this->mMysqlHelper->ToSqlCommand(request, sql))
        {
            return XCode::CallArgsError;
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
        std::string sql;
        if (!this->mMysqlHelper->ToSqlCommand(request, sql))
        {
            return XCode::CallArgsError;
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
        std::string sql;
        if (!this->mMysqlHelper->ToSqlCommand(request, sql))
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

        for (size_t index = 0; index < command->size(); index++)
        {
            response.add_jsons(std::move(command->at(index)));
        }
        return XCode::Successful;
    }
}// namespace Sentry