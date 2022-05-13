//
// Created by mac on 2021/11/28.
//

#include"GateComponent.h"
#include"App/App.h"
#include"NetWork/GateRpcClientContext.h"
#include"Global/ServiceConfig.h"
#include"Task/RpcProxyTask.h"
#include"Component/Rpc/RpcHandlerComponent.h"
#include"GateClientComponent.h"
#include"Component/Rpc/RpcClientComponent.h"
#include"Component/RpcService/LocalServiceComponent.h"
#ifdef __DEBUG__
#include"Pool/MessagePool.h"
#include"google/protobuf/util/json_util.h"
#endif
#include"GateService.h"
namespace Sentry
{

	bool GateComponent::LateAwake()
	{
		this->mTaskComponent = this->GetApp()->GetTaskComponent();
		this->mTimerComponent = this->GetApp()->GetTimerComponent();
		LOG_CHECK_RET_FALSE(this->mGateClientComponent = this->GetComponent<GateClientComponent>());
		return true;
	}

	void GateComponent::OnConnect(const std::string & address)
	{
		unsigned int timerId = this->mTimerComponent->DelayCall(5000,
				&GateComponent::OnSocketTimeout, this, address);
		this->mSocketTimers.emplace(address, timerId);
	}

	void GateComponent::OnSocketTimeout(const std::string & address)
	{
		auto iter = this->mSocketTimers.find(address);
		if(iter != this->mSocketTimers.end())
		{
			this->mSocketTimers.erase(iter);
			this->mGateClientComponent->StartClose(address);
		}
	}

	XCode GateComponent::OnRequest(std::shared_ptr<c2s::Rpc_Request> request)
	{
		const ServiceConfig & rpcConfig = this->GetApp()->GetServiceConfig();
		const RpcInterfaceConfig * config = rpcConfig.GetInterfaceConfig(request->method_name());
		if (config == nullptr || config->Type != "Client")
		{
			LOG_ERROR("call function " << request->method_name() << " not find");
			return XCode::NotFoundRpcConfig;
		}
		if(!config->Request.empty() && !request->has_data())
		{
			return XCode::CallArgsError;
		}
		std::string fullName;
		if(!Any::ParseAnyTypeUrl(request->data().type_url(), &fullName) || fullName != config->Request)
		{
			return XCode::CallArgsError;
		}

		long long userId = 0;
		const std::string &address = request->address();
		if(!this->mGateClientComponent->GetUserId(address, userId))
		{
			return this->Auth(request);
		}
		this->mTaskComponent->Start(&GateComponent::OnUserRequest, this, userId, request);
		return XCode::Successful;
	}

	bool GateComponent::AddUserToken(const std::string& token, long long userId)
	{
		auto iter = this->mUserTokens.find(token);
		if(iter == this->mUserTokens.end())
		{
			this->mUserTokens.emplace(token, userId);
			return true;
		}
		return false;
	}

	XCode GateComponent::Auth(const std::shared_ptr<c2s::Rpc::Request> request)
	{
		//TODO
		//this->mGateClientComponent->AddNewUser(request->address(), userId);
		std::shared_ptr<c2s::Rpc::Response> clientResponse(new c2s::Rpc::Response());

		clientResponse->set_rpc_id(request->rpc_id());
		clientResponse->set_code((int)XCode::Successful);
		this->mGateClientComponent->SendToClient(request->address(), clientResponse);
		this->mTaskComponent->Start([this]()
		{
			std::string address;
			Json::Writer jsonWriter;
			if(this->GetConfig().GetListenerAddress("rpc", address))
			{
				jsonWriter.AddMember("user_id", 0);
				jsonWriter.AddMember("address", address);
				return this->GetComponent<GateService>()->PublishEvent("user_join_event", jsonWriter);
			}
		});
		return XCode::Successful;
	}

	XCode GateComponent::OnResponse(const std::string & address, std::shared_ptr<c2s::Rpc_Response> response)
	{
#ifdef __DEBUG__
		LOG_WARN("**********[client response]**********");
		if (response->has_data())
		{
			std::string json;
			if ( Helper::Proto::GetJson(response->data(), json))
			{
				LOG_WARN("json = " << json);
			}
		}
		LOG_WARN("*****************************************");
#endif
		if (this->mGateClientComponent->SendToClient(address, response))
		{
			return XCode::NetWorkError;
		}
		return XCode::Successful;
	}

	void GateComponent::OnUserRequest(long long userId, std::shared_ptr<c2s::Rpc::Request> request)
	{
		const ServiceConfig& rpcConfig = this->GetApp()->GetServiceConfig();
		const RpcInterfaceConfig* config = rpcConfig.GetInterfaceConfig(request->method_name());
		if (config == nullptr)
		{
			return;
		}
		std::string address;
		LocalServiceComponent* localServerRpc = this->GetComponent<LocalServiceComponent>(config->Service);
		if (!localServerRpc->GetEntityAddress(userId, address) && localServerRpc->AllotAddress(address))
		{
			//TODO
		}
		std::shared_ptr<com::Rpc::Request> requestData(new com::Rpc::Request());
		requestData->set_user_id(userId);
		requestData->set_method_id(config->InterfaceId);
		requestData->mutable_data()->PackFrom(request->data());
		std::shared_ptr<c2s::Rpc::Response> clientResponse(new c2s::Rpc::Response());
		std::shared_ptr<com::Rpc::Response> responseData = localServerRpc->StartCall(address, requestData);
		if(responseData != nullptr)
		{
			clientResponse->set_rpc_id(request->rpc_id());
			clientResponse->set_code(responseData->code());
			clientResponse->set_error_str(responseData->error_str());
			clientResponse->mutable_data()->PackFrom(responseData->data());
			this->mGateClientComponent->SendToClient(request->address(), clientResponse);
		}
	}
}