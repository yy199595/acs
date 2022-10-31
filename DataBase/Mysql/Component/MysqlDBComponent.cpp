//
// Created by zmhy0073 on 2022/7/16.
//

#include"MysqlDBComponent.h"
#include"Client/MysqlClient.h"
#include"Message/user.pb.h"
#include"Config/MysqlConfig.h"
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
    void MysqlDBComponent::CloseClients()
    {
        for(std::shared_ptr<MysqlClient> mysqlClient : this->mMysqlClients)
        {
            mysqlClient->Stop();
        }
        this->mMysqlClients.clear();
    }

	void MysqlDBComponent::OnConnectSuccessful(const std::string& address)
	{
		LOG_INFO("mysql client [" << address << "] auth successful");
	}

	void MysqlDBComponent::OnMessage(const std::string& address, std::shared_ptr<Mysql::Response> message)
	{
		if(!message->IsOk())
		{
			LOG_ERROR("mysql error : " << message->GetError());
		}
		long long taskId = message->TaskId();
		this->OnResponse(taskId, message);
	}

    bool MysqlDBComponent::StartConnectMysql()
	{
        LOG_CHECK_RET_FALSE(MysqlConfig::Inst());
        const MysqlConfig * config = MysqlConfig::Inst();
		for (int index = 0; index < config->MaxCount; index++)
		{
			std::shared_ptr<MysqlClient> mysqlClient
				= std::make_shared<MysqlClient>(this);

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
        return this->Run(this->GetClient(index), command);
    }

    bool MysqlDBComponent::Run(MysqlClient * client, std::shared_ptr<Mysql::ICommand> command)
    {
        std::shared_ptr<MysqlTask> mysqlTask
            = std::make_shared<MysqlTask>(command->GetRpcId(), 0);

        client->SendCommand(command);
        long long key = mysqlTask->GetRpcId();
        std::shared_ptr<Mysql::Response> response = this->AddTask(key, mysqlTask)->Await();
#ifdef __DEBUG__
        long long t1 = Helper::Time::NowMilTime();
        if (response != nullptr && !response->IsOk())
        {
            CONSOLE_LOG_ERROR(response->GetError());
            throw std::logic_error(response->GetError());
        }
        long long t2 = Helper::Time::NowMilTime();
        CONSOLE_LOG_INFO("sql use time = [" << t2 - t1 << "ms]");
#endif
        return response != nullptr && response->IsOk();
    }

    MysqlClient * MysqlDBComponent::GetClient(int index)
    {
        if(index > 0)
        {
            int idx = index % this->mMysqlClients.size();
            return this->mMysqlClients[idx].get();
        }
        std::shared_ptr<MysqlClient> mysqlClient = this->mMysqlClients[0];
        for(size_t x = 0; x < this->mMysqlClients.size(); x++)
        {
            if(this->mMysqlClients[x]->GetTaskCount() <= 5)
            {
                return this->mMysqlClients[x].get();
            }
            if(this->mMysqlClients[x]->GetTaskCount() < mysqlClient->GetTaskCount())
            {
                mysqlClient = this->mMysqlClients[x];
            }
        }
        return mysqlClient.get();
    }
}