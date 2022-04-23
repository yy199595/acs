
#include"ClientComponent.h"
#include"App/App.h"
#include<Util/StringHelper.h>
#include"Network/TcpRpcClient.h"
#include"Task/ClientRpcTask.h"
#include"Network/Http/HttpAsyncRequest.h"
#include"Component/Http/HttpComponent.h"
#include"google/protobuf/util/json_util.h"
#include"Component/Logic/HttpUserService.h"
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

		//LOG_WARN("response json = " << json);
		auto iter = this->mRpcTasks.find(t2->rpc_id());
		if(iter != this->mRpcTasks.end())
		{
			iter->second->SetResult(t2);
			this->mRpcTasks.erase(iter);
		}
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
        this->mHttpComponent = this->GetComponent<HttpComponent>();
        return true;
    }

	void ClientComponent::OnAllServiceStart()
	{
		this->mTaskComponent->Start(&ClientComponent::StartClient, this);
	}

	XCode ClientComponent::Call(const std::string& name)
	{
		std::shared_ptr<c2s::Rpc_Request> requestMessage(new c2s::Rpc_Request());

		requestMessage->set_rpc_id(100);
		requestMessage->set_method_name(name);
		this->mTcpClient->SendToGate(requestMessage);

		TaskSourceShared<c2s::Rpc::Response> taskSource
				= std::make_shared<TaskSource<std::shared_ptr<c2s::Rpc::Response>>>();
		this->mRpcTasks.emplace(100, taskSource);
		std::shared_ptr<c2s::Rpc::Response> response = taskSource->Await();

		return (XCode)response->code();
	}

	XCode ClientComponent::Call(const string& name, const Message& request)
	{
		std::shared_ptr<c2s::Rpc_Request> requestMessage(new c2s::Rpc_Request());

		requestMessage->set_rpc_id(100);
		requestMessage->set_method_name(name);
		requestMessage->mutable_data()->PackFrom(request);
		this->mTcpClient->SendToGate(requestMessage);

		TaskSourceShared<c2s::Rpc::Response> taskSource
				= std::make_shared<TaskSource<std::shared_ptr<c2s::Rpc::Response>>>();
		this->mRpcTasks.emplace(100, taskSource);
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
		if(!this->GetClient("646585122@qq.com", "199595yjz."))
		{
			LOG_ERROR("client error");
			return;
		}
		while(this->mTcpClient->GetSocketProxy()->IsOpen())
		{
			this->Call("GateService.Ping");
		}
	}


	bool ClientComponent::GetClient(const std::string& account, const std::string& passwd)
	{
		//this->mTaskComponent->Sleep(2000);
		std::string url;
		long long userPhoneNumber = 13716061995;
		this->GetConfig().GetMember("url", url);

		Json::Writer jsonWriter;
		jsonWriter.AddMember("password", passwd);
		jsonWriter.AddMember("account", account);
		jsonWriter.AddMember("phone_num", userPhoneNumber);
		std::shared_ptr<Json::Reader> registerResponse(new Json::Reader());

		HttpUserService * httpUserService = this->GetComponent<HttpUserService>();
		if(httpUserService->Post("logic/account/register", jsonWriter, registerResponse) != XCode::Successful)
		{
			std::string error;
			registerResponse->GetMember("error", error);
			LOG_WARN("register " << account << " failure error = " << error);
		}
		else
		{
			LOG_DEBUG("register " << account << " successful");
		}

		Json::Writer loginJsonWriter;
		loginJsonWriter.AddMember("account", account);
		loginJsonWriter.AddMember("password", passwd);
		std::shared_ptr<Json::Reader> loginResponse(new Json::Reader());
		if(httpUserService->Post("logic/account/login", loginJsonWriter, loginResponse) != XCode::Successful)
		{
			std::string error;
			loginResponse->GetMember("error", error);
			LOG_ERROR("login account " << account << " failure error = " << error);
			return false;
		}

		std::string loginToken;
		std::string gateAddress;
		loginResponse->GetMember("data","token", loginToken);
		loginResponse->GetMember("data","address", gateAddress);
		Helper::String::ParseIpAddress(gateAddress, this->mIp, this->mPort);

		IAsioThread& netThread = this->GetApp()->GetTaskScheduler();
		std::shared_ptr<SocketProxy> socketProxy =
			std::make_shared<SocketProxy>(netThread, this->mIp, this->mPort);
		this->mTcpClient = std::make_shared<TcpRpcClient>(socketProxy, this);

		int count = 0;
		while (!this->mTcpClient->ConnectAsync()->Await())
		{
			LOG_ERROR("connect server failure count = " << ++count);
			this->mTaskComponent->Sleep(3000);
		}

		this->mTcpClient->StartReceive();
		LOG_DEBUG("connect " << this->mIp << ':' << this->mPort << " successful");
		std::shared_ptr<c2s::Rpc_Request> requestMessage(new c2s::Rpc_Request());

		c2s::GateLogin::Request loginRequest;
		loginRequest.set_token(loginToken);
		if(this->Call("GateService.Login", loginRequest) != XCode::Successful)
		{
			return false;
		}
		return true;
	}
}
