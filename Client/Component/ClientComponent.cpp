
#include"Object/App.h"
#include"ClientComponent.h"
#include<Util/StringHelper.h>
#include"Network/TcpRpcClient.h"
#include"Network/ClientRpcTask.h"
#include"Other/ElapsedTimer.h"
#include"Http/HttpAsyncRequest.h"
#include"Http/Component/HttpClientComponent.h"
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
        std::string json;
        util::MessageToJsonString(*t1, &json);
        LOG_ERROR("request json = ", json);
    }

    void ClientComponent::OnResponse(std::shared_ptr<c2s::Rpc_Response> t2)
    {
        std::string json;
        util::MessageToJsonString(*t2, &json);
        LOG_WARN("response json = ", json);
        auto iter = this->mRpcTasks.find(t2->rpc_id());
        if(iter != this->mRpcTasks.end())
        {
            iter->second->SetResult(t2);
            this->mRpcTasks.erase(iter);
        }
    }

	bool ClientComponent::Awake()
    {
        return true;
    }

    bool ClientComponent::LateAwake()
    {
        this->mTaskComponent = this->GetComponent<TaskComponent>();
        this->mTimerComponent = this->GetComponent<TimerComponent>();
        this->mHttpComponent = this->GetComponent<HttpClientComponent>();
        return true;
    }

	void ClientComponent::OnStart()
	{
        string host;
        const ServerConfig & config = App::Get().GetConfig();
        config.GetValue("http", "account", host);
        std::string loginUrl = host + "/logic/account/login";
        std::string registerUrl = host + "/logic/account/register";

        RapidJsonWriter jsonWriter;
        jsonWriter.Add("password", "199595yjz.");
        jsonWriter.Add("account", "646585122@qq.com");
        jsonWriter.Add("phone_num", (long long)13716061995);
        auto registerResponse = this->mHttpComponent->Post(registerUrl, jsonWriter);
        std::shared_ptr<RapidJsonReader> rapidJsonReader = registerResponse->ToJsonReader();


        RapidJsonWriter loginJsonWriter;
        loginJsonWriter.Add("password", "199595yjz.");
        loginJsonWriter.Add("account","646585122@qq.com");
        auto loginResponse = this->mHttpComponent->Post(loginUrl, loginJsonWriter);

        std::shared_ptr<RapidJsonReader> loginJsonResponse = loginResponse->ToJsonReader();

        loginJsonResponse->TryGetValue("gate_ip", this->mIp);
        loginJsonResponse->TryGetValue("gate_port", this->mPort);
        LOG_INFO(loginResponse->GetContent());

		IAsioThread & netThread = App::Get().GetTaskScheduler();
        std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(netThread, "Client"));
		this->mTcpClient = std::make_shared<TcpRpcClient>(socketProxy, this);

        int count = 0;
        while(!this->mTcpClient->ConnectAsync(this->mIp, this->mPort)->Await())
        {
            LOG_ERROR("connect server failure count = ", ++count);
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
            c2s::GateLogin gateLoginData;

            //this->Call("GateService.Login", )


            this->mTaskComponent->Sleep(10);

        }
	}

    XCode ClientComponent::Call(const std::string &name, std::shared_ptr<Message> response)
    {
        return XCode::Successful;
    }

    XCode ClientComponent::Call(const std::string &name, const Message &message, std::shared_ptr<Message> response)
    {
        std::shared_ptr<c2s::Rpc_Request> requestMessage(new c2s::Rpc_Request());
        std::shared_ptr<TaskSource<std::shared_ptr<c2s::Rpc_Response>>> rpcTask(new TaskSource<std::shared_ptr<c2s::Rpc_Response>>());

        requestMessage->set_method_name(name);
        requestMessage->mutable_data()->CopyFrom(message);
        requestMessage->set_rpc_id(rpcTask->GetTaskId());
        this->mTcpClient->SendToGate(requestMessage);
        this->mRpcTasks.emplace(rpcTask->GetTaskId(), rpcTask);
        std::shared_ptr<c2s::Rpc_Response> responseData = rpcTask->Await();
        if(responseData->code() != (int)XCode::Successful)
        {
            return (XCode)responseData->code();
        }
        const Any & any = responseData->data();
        if(!any.UnpackTo(response.get()))
        {
            return XCode::ParseMessageError;
        }
        return XCode::Successful;
    }

	void ClientComponent::OnTimeout(long long rpcId)
	{
		auto iter = this->mRpcTasks.find(rpcId);
		if (iter != this->mRpcTasks.end())
		{
			auto rpcTask = iter->second;
			this->mRpcTasks.erase(iter);
			rpcTask->SetResult(nullptr);
		}
	}
}
