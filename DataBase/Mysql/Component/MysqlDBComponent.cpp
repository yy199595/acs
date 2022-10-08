//
// Created by zmhy0073 on 2022/7/16.
//

#include"MysqlDBComponent.h"
#include"Client/MysqlClient.h"
#include"Message/user.pb.h"
namespace Sentry
{
    MysqlTask::MysqlTask(long long taskId, int ms)
        : IRpcTask<Mysql::Response>(ms)
    {
        this->mTaskId = taskId;
    }

    void MysqlTask::OnTimeout()
    {
        this->mTask.SetResult(nullptr);
    }

    void MysqlTask::OnResponse(std::shared_ptr<Mysql::Response> response)
    {
        this->mTask.SetResult(response);
    }
}

namespace Sentry
{
    bool MysqlDBComponent::LoadConfig()
	{
		const ServerConfig& config = this->GetConfig();
		config.GetMember("mysql", "ip", this->mConfig.mIp);
		config.GetMember("mysql", "port", this->mConfig.mPort);
		config.GetMember("mysql", "user", this->mConfig.mUser);
		config.GetMember("mysql", "count", this->mConfig.mMaxCount);
		config.GetMember("mysql", "passwd", this->mConfig.mPassword);
		this->mConfig.mAddress = fmt::format("{0}:{1}", this->mConfig.mIp, this->mConfig.mPort);
		return true;
	}

    bool MysqlDBComponent::StartConnectMysql()
	{
        if(!this->LoadConfig())
        {
            return false;
        }
		for (int index = 0; index < this->mConfig.mMaxCount; index++)
		{
			std::shared_ptr<MysqlClient> mysqlClient
				= std::make_shared<MysqlClient>(this->mConfig, this);

			mysqlClient->Start();
			this->mMysqlClients.emplace_back(std::move(mysqlClient));
		}
		try
		{
			std::shared_ptr<Mysql::SqlCommand> command
				= std::make_shared<Mysql::SqlCommand>("DROP TABLE user.account_info");
			this->Run(this->GetClient(), command);
		}
		catch (std::exception& e)
		{
			CONSOLE_LOG_ERROR(e.what());
		}
		return this->Ping(0);
	}

    bool MysqlDBComponent::Ping(int index)
    {
        std::shared_ptr<Mysql::PingCommand> command
            = std::make_shared<Mysql::PingCommand>();
        std::shared_ptr<MysqlClient> mysqlCLient = this->GetClient(index);
        return this->Run(mysqlCLient, command);
    }

    bool MysqlDBComponent::Run(
        std::shared_ptr<MysqlClient> client, std::shared_ptr<Mysql::ICommand> command)
    {
        std::shared_ptr<MysqlTask> mysqlTask
            = std::make_shared<MysqlTask>(command->GetRpcId(), 0);

        client->SendCommand(command);
        std::shared_ptr<Mysql::Response> response = this->AddTask(mysqlTask)->Await();
#ifdef __DEBUG__
        long long t1 = Helper::Time::GetNowMilTime();
        if (response != nullptr && !response->IsOk())
        {
            CONSOLE_LOG_ERROR(response->GetError());
            throw std::logic_error(response->GetError());
        }
        long long t2 = Helper::Time::GetNowMilTime();
        CONSOLE_LOG_INFO("sql use time = [" << t2 - t1 << "ms]");
#endif
        return response != nullptr && response->IsOk();
    }

    std::shared_ptr<MysqlClient> MysqlDBComponent::GetClient(int index)
    {
        if(index > 0)
        {
            int idx = index % this->mMysqlClients.size();
            return this->mMysqlClients[idx];
        }
        std::shared_ptr<MysqlClient> mysqlClient = this->mMysqlClients[0];
        for(size_t x = 0; x < this->mMysqlClients.size(); x++)
        {
            if(this->mMysqlClients[x]->GetTaskCount() <= 5)
            {
                return this->mMysqlClients[x];
            }
            if(this->mMysqlClients[x]->GetTaskCount() < mysqlClient->GetTaskCount())
            {
                mysqlClient = this->mMysqlClients[x];
            }
        }
        return mysqlClient;
    }
}