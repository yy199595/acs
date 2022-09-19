//
// Created by zmhy0073 on 2022/7/16.
//

#include"MysqlDBComponent.h"
#include"Client/MysqlClient.h"
#include"Message/user.pb.h"
namespace Sentry
{
    MysqlTask::MysqlTask(int taskId, int ms)
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
    bool MysqlDBComponent::LateAwake()
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

    bool MysqlDBComponent::OnStart()
	{
		for (int index = 0; index < this->mConfig.mMaxCount; index++)
		{
			std::shared_ptr<MysqlClient> mysqlClient
				= std::make_shared<MysqlClient>(this->mConfig, this);
			this->mMysqlClients.emplace_back(std::move(mysqlClient));
		}

        std::shared_ptr<user::account_info> account(new user::account_info());


        std::shared_ptr<Mysql::CreateTabCommand> command
            = std::make_shared<Mysql::CreateTabCommand>(account, 1);
        std::shared_ptr<Mysql::Response> response = this->Run(this->GetClient(0), command);

        return this->Ping(0);
	}

    bool MysqlDBComponent::Ping(int index)
    {
        int id = this->mNumberPool.Pop();
        std::shared_ptr<Mysql::PingCommand> command
            = std::make_shared<Mysql::PingCommand>(id);
        std::shared_ptr<MysqlClient> mysqlCLient = this->GetClient(index);
        std::shared_ptr<Mysql::Response> response = this->Run(mysqlCLient, command);
        return response != nullptr && !response->HasError();
    }

    std::shared_ptr<Mysql::Response> MysqlDBComponent::Run(
        std::shared_ptr<MysqlClient> client, std::shared_ptr<Mysql::ICommand> command)
    {
        std::shared_ptr<MysqlTask> mysqlTask
            = std::make_shared<MysqlTask>(command->GetRpcId(), 0);
        this->AddTask(mysqlTask);
        client->SendCommand(command);
#ifdef __DEBUG__
        long long t1 = Helper::Time::GetNowMilTime();
        std::shared_ptr<Mysql::Response> response = mysqlTask->Await();
        if(response->HasError())
        {
            CONSOLE_LOG_ERROR(response->GetError());
        }
        long long t2 = Helper::Time::GetNowMilTime();
        CONSOLE_LOG_INFO("sql use time = [" << t2 - t1 << "ms]");
        return response;
#else
        return mysqlTask->Await();
#endif
    }

    void MysqlDBComponent::OnDelTask(long long taskId, RpcTask task)
    {
        this->mNumberPool.Push((int) taskId);
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