
#include"ClientComponent.h"
#include"App/App.h"
#include<Util/StringHelper.h>
#include"Network/TcpRpcClient.h"
#include"Task/ClientRpcTask.h"
#include"Other/ElapsedTimer.h"
#include"Network/Http/HttpAsyncRequest.h"
#include"Component/Http/HttpClientComponent.h"
#include"google/protobuf/util/json_util.h"
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
		//this->mRpcTasks.emplace(rpcId, task);
		return ms > 0 ? this->mTimerComponent->AsyncWait(ms, &ClientComponent::OnTimeout, this, rpcId) : 0;
	}

    void ClientComponent::OnRequest(std::shared_ptr<c2s::Rpc_Request> t1)
    {
        std::string json;
        util::MessageToJsonString(*t1, &json);
        LOG_ERROR("request json = " << json);
    }

    void ClientComponent::OnResponse(std::shared_ptr<c2s::Rpc_Response> t2)
    {
        std::string json;
        util::MessageToJsonString(*t2, &json);
        LOG_WARN("response json = " << json);
//        auto iter = this->mRpcTasks.find(t2->rpc_id());
//        if(iter != this->mRpcTasks.end())
//        {
//            iter->second->SetResult(t2);
//            this->mRpcTasks.erase(iter);
//        }
    }

    bool ClientComponent::LateAwake()
    {
        this->mTaskComponent = this->GetComponent<TaskComponent>();
        this->mTimerComponent = this->GetComponent<TimerComponent>();
        this->mHttpComponent = this->GetComponent<HttpClientComponent>();
        return true;
    }

	void ClientComponent::OnAllServiceStart()
	{
		this->mTaskComponent->Start(&ClientComponent::StartClient, this);
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
        //this->mRpcTasks.emplace(rpcTask->GetTaskId(), rpcTask);
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
	void ClientComponent::StartClient()
	{
		//this->mTaskComponent->Sleep(2000);

		std::string loginUrl;
		std::string registerUrl;
		std::string userAccount = "646585122@qq.com";
		std::string userPassword = "199595yjz.";
		long long userPhoneNumber = 13716061995;
		const ServerConfig& config = App::Get()->GetConfig();
		LOG_CHECK_RET(config.GetMember("url", "login", loginUrl));
		LOG_CHECK_RET(config.GetMember("url", "register", registerUrl));

		Json::Writer jsonWriter;
		jsonWriter.AddMember("password", userPassword);
		jsonWriter.AddMember("account", userAccount);
		jsonWriter.AddMember("phone_num", userPhoneNumber);
		std::shared_ptr<HttpAsyncResponse> registerResponse = this->mHttpComponent->Post(registerUrl, jsonWriter);
		std::shared_ptr<Json::Reader> registerReader = registerResponse->ToJsonReader();

		XCode code = XCode::Successful;
		LOG_CHECK_FATAL(registerReader->GetMember("code", code));
		if(code != XCode::Successful)
		{
			std::string error;
			registerReader->GetMember("error", error);
			LOG_WARN("register " << userAccount << " failure error = " << error);
		}
		else
		{
			LOG_DEBUG("register " << userAccount << " successful");
		}

		Json::Writer loginJsonWriter;
		loginJsonWriter.AddMember("account", userAccount);
		loginJsonWriter.AddMember("password", userPassword);
		std::shared_ptr<HttpAsyncResponse> loginResponse = this->mHttpComponent->Post(loginUrl, loginJsonWriter);

		std::shared_ptr<Json::Reader> loginJsonResponse = loginResponse->ToJsonReader();
		LOG_CHECK_FATAL(registerReader->GetMember("code", code));
		if(code != XCode::Successful)
		{
			std::string error;
			registerReader->GetMember("error", error);
			LOG_ERROR("login account " << userAccount << " failure error = " << error);
			return;
		}
		LOG_DEBUG(userAccount << " login successful");
		LOG_CHECK_RET(loginJsonResponse->GetMember("gate_ip", this->mIp));
		LOG_CHECK_RET(loginJsonResponse->GetMember("gate_port", this->mPort));

		std::string content = loginResponse->GetContent();
		IAsioThread& netThread = App::Get()->GetTaskScheduler();
		std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(netThread, "Client"));
		this->mTcpClient = std::make_shared<TcpRpcClient>(socketProxy, this);

		int count = 0;
		while (!this->mTcpClient->ConnectAsync(this->mIp, this->mPort)->Await())
		{
			LOG_ERROR("connect server failure count = " << ++count);
			this->mTaskComponent->Sleep(1000);
		}

		this->mTcpClient->StartReceive();
		LOG_DEBUG("connect " << this->mIp << ':' << this->mPort << " successful");
		std::shared_ptr<c2s::Rpc_Request> requestMessage(new c2s::Rpc_Request());
		const std::string method = "HttpUserService.Register";

		c2s::AccountRegister_Request registerRequest;
		registerRequest.set_account("112233@qq.com");
		registerRequest.set_password("==================");

		requestMessage->set_rpc_id(1);
		requestMessage->set_method_name(method);
		requestMessage->mutable_data()->PackFrom(registerRequest);

		while (this->mTcpClient->IsOpen())
		{
			ElapsedTimer timer;
			c2s::GateLogin gateLoginData;

			//this->Call("GateService.Login", )

			this->mTaskComponent->Sleep(10);

		}
	}
}
