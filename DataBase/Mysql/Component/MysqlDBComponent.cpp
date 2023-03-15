//
// Created by zmhy0073 on 2022/7/16.
//

#include"MysqlDBComponent.h"
#include"Client/MysqlClient.h"
#include"Message/user.pb.h"
#include"Config/MysqlConfig.h"
namespace Sentry
{
    MysqlTask::MysqlTask(int taskId)
        : IRpcTask<Mysql::Response>(taskId)
    {

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
        for(std::unique_ptr<MysqlClient> & mysqlClient : this->mMysqlClients)
        {
            mysqlClient->Stop();
        }
        this->mMysqlClients.clear();
    }

	void MysqlDBComponent::OnConnectSuccessful(const std::string& address)
	{
		LOG_INFO("mysql client [" << address << "] auth successful");
	}

	void MysqlDBComponent::OnMessage(std::shared_ptr<Mysql::Response> message)
	{
		if(!message->IsOk())
		{
			LOG_ERROR("mysql error : " << message->GetError());
		}
		long long taskId = message->TaskId();
		this->OnResponse(taskId, message);
	}

    size_t MysqlDBComponent::MakeMysqlClient()
    {
        std::unique_ptr<MysqlClient> mysqlClient
                = std::make_unique<MysqlClient>(this);

        mysqlClient->Start();
        this->mClients.push(mysqlClient.get());
        this->mMysqlClients.emplace_back(std::move(mysqlClient));
        return this->mMysqlClients.size() - 1;
    }

    bool MysqlDBComponent::Ping(int index)
    {
        std::shared_ptr<Mysql::PingCommand> command
            = std::make_shared<Mysql::PingCommand>();
		return this->Run(index, command)->IsOk();
    }

	std::shared_ptr<Mysql::Response> MysqlDBComponent::Run(std::shared_ptr<Mysql::ICommand> command)
	{
		int id = this->mNumerPool.Pop();
		{
			command->SetRpcId(id);
			this->Send(std::move(command));
		}
		std::shared_ptr<MysqlTask> mysqlTask = std::make_shared<MysqlTask>(id);
		std::shared_ptr<Mysql::Response> response = this->AddTask(id, mysqlTask)->Await();
		if (response != nullptr && !response->IsOk())
		{
			LOG_ERROR(response->GetError());
		}
		return response;
	}

	std::shared_ptr<Mysql::Response> MysqlDBComponent::Run(int index, std::shared_ptr<Mysql::ICommand> command)
    {
		int id = this->mNumerPool.Pop();
		{
			command->SetRpcId(id);
			this->Send(index, std::move(command));
		}
		std::shared_ptr<MysqlTask> mysqlTask = std::make_shared<MysqlTask>(id);
		std::shared_ptr<Mysql::Response> response = this->AddTask(id, mysqlTask)->Await();
		if (response != nullptr && !response->IsOk())
		{
			LOG_ERROR(response->GetError());
		}
        return response;
    }

	bool MysqlDBComponent::Send(int index, std::shared_ptr<Mysql::ICommand> command)
	{
		if(this->mMysqlClients.empty())
		{
			return false;
		}
		int idx = index % this->mMysqlClients.size();
		MysqlClient* mysqlClient = this->mMysqlClients[idx].get();;
		{
			mysqlClient->SendCommand(command);
		}
		return true;
	}
	bool MysqlDBComponent::Send(std::shared_ptr<Mysql::ICommand> command)
	{
		if(this->mMysqlClients.empty())
		{
			return false;
		}
		MysqlClient* mysqlClient = this->mClients.front();
		{
			this->mClients.pop();
			this->mClients.push(mysqlClient);
		}
		mysqlClient->SendCommand(command);
		return true;
	}
}