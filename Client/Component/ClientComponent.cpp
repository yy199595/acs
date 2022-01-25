
#include"Object/App.h"
#include"ClientComponent.h"
#include<Util/StringHelper.h>
#include"Network/TcpRpcClient.h"
#include"Network/ClientRpcTask.h"
#include"Other/ElapsedTimer.h"
namespace Client
{
	ClientComponent::ClientComponent()
	{		
		this->mTcpClient = nullptr;
        this->mTimerComponent = nullptr;
	}

	unsigned int ClientComponent::AddRpcTask(std::shared_ptr<ClientRpcTask> task, int ms)
	{
		long long rpcId = task->GetRpcTaskId();
		this->mRpcTasks.emplace(rpcId, task);
		return ms > 0 ? this->mTimerComponent->AsyncWait(ms, &ClientComponent::OnTimeout, this, rpcId) : 0;
	}

    void ClientComponent::OnRequest(std::shared_ptr<c2s::Rpc_Request> t1)
    {

    }

    void ClientComponent::OnResponse(std::shared_ptr<c2s::Rpc_Response> t2)
    {

    }

	bool ClientComponent::Awake()
    {
		return true;
    }

    bool ClientComponent::LateAwake()
    {
        this->mTaskComponent = this->GetComponent<TaskComponent>();
        this->mTimerComponent = this->GetComponent<TimerComponent>();
        return true;
    }

	void ClientComponent::OnStart()
	{
		IAsioThread & netThread = App::Get().GetTaskScheduler();
        std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(netThread, "Client"));
		this->mTcpClient = std::make_shared<TcpRpcClient>(socketProxy, this);

        while(!this->mTcpClient->ConnectAsync("192.168.8.114", 1995)->Await())
        {
            LOG_ERROR("connect server failure");
            this->mTaskComponent->Sleep(1000);
        }

		this->mTcpClient->StartReceive();
		LOG_DEBUG("connect server successful");

        std::shared_ptr<c2s::Rpc_Request> requestMessage(new c2s::Rpc_Request());
		const std::string method = "AccountService.Register";

		c2s::AccountRegister_Request registerRequest;
        registerRequest.set_account("112233@qq.com");
        registerRequest.set_password("==================");


		requestMessage->set_rpc_id(1);
		requestMessage->set_method_name(method);
		requestMessage->mutable_data()->PackFrom(registerRequest);

        while(this->mTcpClient->IsOpen())
        {
            ElapsedTimer timer;
            //this->mTaskComponent->Sleep(10);
            if(this->mTcpClient->SendToGate(requestMessage)->Await())
            {
                LOG_INFO("use time = ", timer.GetMs() , "ms");
            }
        }
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
}
