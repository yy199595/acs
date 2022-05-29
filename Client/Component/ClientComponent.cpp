
#include"ClientComponent.h"
#include"App/App.h"
#include<Util/StringHelper.h>
#include"Network/TcpRpcClientContext.h"
#include"Task/ClientRpcTask.h"
#include"Pool/MessagePool.h"
#include"Network/Http/HttpAsyncRequest.h"
#include"Component/Http/HttpComponent.h"
#include"google/protobuf/util/json_util.h"
#include"Component/Logic/HttpUserService.h"
namespace Client
{
	ClientComponent::ClientComponent()
	{
        this->mTimerComponent = nullptr;
	}

	unsigned int ClientComponent::AddRpcTask(std::shared_ptr<ClientRpcTask> task, int ms)
	{
		long long rpcId = task->GetRpcTaskId();
		//this->mRpcTasks.emplace(rpcId, task);
		return ms > 0 ? this->mTimerComponent->DelayCall(ms, &ClientComponent::OnTimeout, this, rpcId) : 0;
	}

    void ClientComponent::OnRequest(std::shared_ptr<c2s::Rpc::Call> t1)
    {
#ifdef __CLIENT_RPC_DEBUG_LOG__
        std::string json;
		LOG_INFO("========== call client ==========");
		LOG_INFO("func = " << t1->func());
		if(Helper::Proto::GetJson(t1->data(), json))
		{
			LOG_INFO("json = " << json);
		}
		LOG_INFO("=================================");
#endif
	}

    void ClientComponent::OnResponse(std::shared_ptr<c2s::Rpc::Response> t2)
    {
        std::string json;
        util::MessageToJsonString(*t2, &json);

		LOG_WARN("response json = " << json);
		auto iter = this->mRpcTasks.find(t2->rpc_id());
		if(iter != this->mRpcTasks.end())
		{
			iter->second->SetResult(t2);
			this->mRpcTasks.erase(iter);
			return;
		}
		LOG_ERROR("not find rpc task id = " << t2->rpc_id());
    }

    bool ClientComponent::LateAwake()
    {
		this->mHttpComponent = this->GetComponent<HttpComponent>();
		this->mTaskComponent = this->GetComponent<TaskComponent>();
        this->mTimerComponent = this->GetComponent<TimerComponent>();
        return true;
    }

	void ClientComponent::OnAllServiceStart()
	{
		this->mTaskComponent->Start(&ClientComponent::StartClient, this, std::string("646585121@qq.com"));
//		this->mTaskComponent->Start(&ClientComponent::StartClient, this, std::string("646585122@qq.com"));
//		this->mTaskComponent->Start(&ClientComponent::StartClient, this, std::string("646585123@qq.com"));
//		this->mTaskComponent->Start(&ClientComponent::StartClient, this, std::string("646585124@qq.com"));
//		this->mTaskComponent->Start(&ClientComponent::StartClient, this, std::string("646585125@qq.com"));
//		this->mTaskComponent->Start(&ClientComponent::StartClient, this, std::string("646585126@qq.com"));
//		this->mTaskComponent->Start(&ClientComponent::StartClient, this, std::string("646585127@qq.com"));
//		this->mTaskComponent->Start(&ClientComponent::StartClient, this, std::string("646585128@qq.com"));
//		this->mTaskComponent->Start(&ClientComponent::StartClient, this, std::string("646585129@qq.com"));
	}

	XCode ClientComponent::Call(const std::string& name)
	{
		std::shared_ptr<c2s::Rpc_Request> requestMessage(new c2s::Rpc_Request());
		TaskSourceShared<c2s::Rpc::Response> taskSource
				= std::make_shared<TaskSource<std::shared_ptr<c2s::Rpc::Response>>>();

		requestMessage->set_method_name(name);
		requestMessage->set_rpc_id(taskSource->GetTaskId());
		this->GetCurrentRpcClient()->SendToServer(requestMessage);
		this->mRpcTasks.emplace(taskSource->GetTaskId(), taskSource);
		std::shared_ptr<c2s::Rpc::Response> response = taskSource->Await();

		return (XCode)response->code();
	}

	XCode ClientComponent::Call(const string& name, const Message& request)
	{
		std::shared_ptr<c2s::Rpc_Request> requestMessage(new c2s::Rpc_Request());

		TaskSourceShared<c2s::Rpc::Response> taskSource
				= std::make_shared<TaskSource<std::shared_ptr<c2s::Rpc::Response>>>();

		requestMessage->set_method_name(name);
		requestMessage->set_rpc_id(taskSource->GetTaskId());
		requestMessage->mutable_data()->PackFrom(request);
		this->GetCurrentRpcClient()->SendToServer(requestMessage);

		this->mRpcTasks.emplace(requestMessage->rpc_id(), taskSource);
		std::shared_ptr<c2s::Rpc::Response> response = taskSource->Await();

		return (XCode)response->code();
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
        requestMessage->set_rpc_id(rpcTask->GetTaskId());
		requestMessage->mutable_data()->CopyFrom(message);

		this->mRpcTasks.emplace(rpcTask->GetTaskId(), rpcTask);
		this->GetCurrentRpcClient()->SendToServer(requestMessage);
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
//		auto iter = this->mRpcTasks.find(rpcId);
//		if (iter != this->mRpcTasks.end())
//		{
//			auto rpcTask = iter->second;
//			this->mRpcTasks.erase(iter);
//			rpcTask->SetResult(nullptr);
//		}
	}

	std::shared_ptr<TcpRpcClientContext> ClientComponent::GetCurrentRpcClient()
	{
		unsigned int contextId = this->mTaskComponent->GetContextId();
		auto iter = this->mClients.find(contextId);
		return iter != this->mClients.end() ? iter->second : nullptr;
	}

	void ClientComponent::StartClient(const std::string & account)
	{
		if(!this->GetClient(account, "199595yjz."))
		{
			LOG_ERROR(account << " login error");
			return;
		}
		LOG_ERROR(account << " login successful");
		while(this->GetCurrentRpcClient()->IsOpen())
		{
			this->Call("GateService.Ping");
			//LOG_ERROR("%%%%%%%%%%%%%%%%%%%%");

			c2s::Chat::Request chatMessage;
			chatMessage.set_message("hello");
			this->Call("ChatService.Chat", chatMessage);

			this->mTaskComponent->Sleep(10000);

		}
	}


	bool ClientComponent::GetClient(const std::string& account, const std::string& passwd)
	{
		std::string url;
		long long userPhoneNumber = 13716061995;
		this->GetConfig().GetMember("url", url);

		Json::Writer jsonWriter;
		jsonWriter.AddMember("password", passwd);
		jsonWriter.AddMember("account", account);
		jsonWriter.AddMember("phone_num", userPhoneNumber);

		std::shared_ptr<Json::Reader> loginResponse(new Json::Reader());
		long long timerId = this->mTimerComponent->DelayCall(5.0f, [account]()
		{
			LOG_ERROR(account << " register time out");
		});
		std::string registerData;
		jsonWriter.WriterStream(registerData);
		string url2 = url + "/logic/account/login";
		string url1 = url + "/logic/account/register";
		std::shared_ptr<HttpAsyncResponse> response1 = this->mHttpComponent->Post(url1, registerData);
		std::shared_ptr<HttpAsyncResponse> response2 = this->mHttpComponent->Post(url2, registerData);
		this->mTimerComponent->CancelTimer(timerId);
		if(response2 == nullptr || !loginResponse->ParseJson(response2->GetContent()))
		{
			return false;
		}

		std::string loginToken;
		std::string gateAddress;
		loginResponse->GetMember("data","token", loginToken);
		loginResponse->GetMember("data","address", gateAddress);
		Helper::String::ParseIpAddress(gateAddress, this->mIp, this->mPort);

		assert(!this->mIp.empty() && this->mPort > 0);
		IAsioThread& netThread = this->GetApp()->GetTaskScheduler();
		std::shared_ptr<SocketProxy> socketProxy =
			std::make_shared<SocketProxy>(netThread, this->mIp, this->mPort);
		std::shared_ptr<TcpRpcClientContext> tcpRpcClient = std::make_shared<TcpRpcClientContext>(socketProxy, this);

		int count = 0;
		while (!tcpRpcClient->ConnectAsync()->Await())
		{
			LOG_ERROR("connect server failure count = " << ++count);
			this->mTaskComponent->Sleep(3000);
		}

		tcpRpcClient->StartReceive();
		unsigned int id = this->mTaskComponent->GetContextId();
		this->mClients.emplace(id, tcpRpcClient);
		LOG_DEBUG("connect " << this->mIp << ':' << this->mPort << " successful");
		std::shared_ptr<c2s::Rpc_Request> requestMessage(new c2s::Rpc_Request());

		c2s::GateAuth::Request loginRequest;
		loginRequest.set_token(loginToken);
		return this->Call("GateService.Auth", loginRequest) == XCode::Successful;
	}
}
