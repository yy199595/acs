﻿
#include"Core/App.h"
#include"ClientComponent.h"
#include<Util/StringHelper.h>
#include"Network/TcpRpcClient.h"
#include"Scene/TaskPoolComponent.h"
#include"Network/ClientRpcTask.h"
#include"Other/ElapsedTimer.h"
namespace Client
{
	ClientComponent::ClientComponent()
	{		
		this->mTcpClient = nullptr;
		this->mTaskComponent = nullptr;
        this->mTimerComponent = nullptr;
	}

	void ClientComponent::StartClose(long long id)
	{

	}

	void ClientComponent::OnRequest(c2s::Rpc_Request * request)
	{

	}

	void ClientComponent::OnResponse(c2s::Rpc_Response * response)
	{
		long long rpcId = response->rpcid();
		auto iter = this->mRpcTasks.find(rpcId);
		if (iter != this->mRpcTasks.end())
		{
			auto rpcTask = iter->second;
			this->mRpcTasks.erase(iter);
			rpcTask->OnResponse(response);
		}
	}

	void ClientComponent::OnCloseSocket(long long id, XCode code)
	{
		
	}

	unsigned int ClientComponent::AddRpcTask(std::shared_ptr<ClientRpcTask> task, int ms)
	{
		long long rpcId = task->GetRpcTaskId();
		this->mRpcTasks.emplace(rpcId, task);
		return ms > 0 ? this->mTimerComponent->AsyncWait(ms, &ClientComponent::OnTimeout, this, rpcId) : 0;
	}

	bool ClientComponent::Awake()
    {
		this->mTimerComponent = this->GetComponent<TimerComponent>();
		this->mTaskComponent = this->GetComponent<TaskPoolComponent>();
		return true;
    }

	void ClientComponent::OnStart()
	{
		NetWorkThread & netThread = this->mTaskComponent->AllocateNetThread();
		this->mTcpClient = new TcpRpcClient(new SocketProxy(netThread, "Client"), this);
		if (!this->mTcpClient->AwaitConnect("192.168.8.114", 1995))
		{
			LOG_FATAL("connect server failure");
			return;
		}
		this->mTcpClient->StartReceive();
		LOG_DEBUG("connect server successful");

        ElapsedTimer timer;
		auto requestMessage = new c2s::Rpc_Request();
		const std::string method = "AccountService.Register";

		c2s::AccountRegister_Request registerRequest;
        registerRequest.set_account("112233@qq.com");
        registerRequest.set_passwd("==================");


		requestMessage->set_rpcid(1);
		requestMessage->set_methodname(method);
		requestMessage->mutable_data()->PackFrom(registerRequest);
		std::shared_ptr<ClientRpcTask> rpcTask(new ClientRpcTask(method, 1));
		this->mTcpClient->StartSendProtoData(requestMessage);

		auto response = rpcTask->AwaitGetData<c2s::AccountLogin_Response>();

		if (response != nullptr)
		{

		}

        LOG_INFO("use time = " << timer.GetMs() << "ms");
		
	}
	void ClientComponent::OnTimeout(long long rpcId)
	{
		auto iter = this->mRpcTasks.find(rpcId);
		if (iter != this->mRpcTasks.end())
		{
			auto rpcTask = iter->second;
			this->mRpcTasks.erase(iter);
			rpcTask->OnResponse(nullptr);
		}
	}

    bool ClientComponent::LateAwake()
    {
        return true;
    }
}