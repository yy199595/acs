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
#include"Component/Redis/MainRedisComponent.h"
namespace Sentry
{

	bool GateComponent::LateAwake()
	{
		this->mTaskComponent = this->GetApp()->GetTaskComponent();
		this->mTimerComponent = this->GetApp()->GetTimerComponent();
		this->mRedisComponent = this->GetComponent<MainRedisComponent>();
		LOG_CHECK_RET_FALSE(this->mGateClientComponent = this->GetComponent<GateClientComponent>());
		return true;
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
		if(!config->Request.empty())
		{
			std::string fullName;
			if(!request->has_data() || !Any::ParseAnyTypeUrl(
				request->data().type_url(), &fullName) || fullName != config->Request)
			{
				return XCode::CallArgsError;
			}
		}
		this->mTaskComponent->Start(&GateComponent::OnUserRequest, this, request);
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

	void GateComponent::OnUserRequest(std::shared_ptr<c2s::Rpc::Request> request)
	{
		std::shared_ptr<c2s::Rpc::Response> response
			= std::make_shared<c2s::Rpc::Response>();
		XCode code = this->HandlerRequest(request, response);
		if (code == XCode::NetActiveShutdown)
		{
			this->mGateClientComponent->StartClose(request->address());
			return;
		}
		response->set_code((int)code);
		response->set_rpc_id(request->rpc_id());
		this->mGateClientComponent->SendToClient(request->address(), response);
	}

	XCode GateComponent::HandlerRequest(std::shared_ptr<c2s::Rpc::Request> request, std::shared_ptr<c2s::Rpc::Response> response)
	{
		long long userId = 0;
		const ServiceConfig& rpcConfig = this->GetApp()->GetServiceConfig();
		const RpcInterfaceConfig* config = rpcConfig.GetInterfaceConfig(request->method_name());
		if (!this->mGateClientComponent->GetUserId(request->address(), userId)) //没有验证
		{
			GateService* gateService = this->GetComponent<GateService>(config->Service);
			std::shared_ptr<c2s::GateAuth::Request> loginRequest(new c2s::GateAuth::Request());
			if (gateService != nullptr && gateService->IsStartService() && request->data().UnpackTo(loginRequest.get()))
			{
				loginRequest->set_address(request->address());
				return gateService->Auth(*loginRequest);
			}
			return XCode::NetActiveShutdown;
		}
		std::string address;
		std::shared_ptr<com::Rpc::Request> requestData(new com::Rpc::Request());
		LocalServiceComponent* localServerRpc = this->GetComponent<LocalServiceComponent>(config->Service);
		if (!localServerRpc->GetEntityAddress(userId, address))
		{
			Json::Writer getJson;
			getJson.AddMember("user_id", userId);
			getJson.AddMember("service", config->Service);
			std::shared_ptr<Json::Reader> getResponse(new Json::Reader());
			if (!this->mRedisComponent->CallLua("user.get_address",
				getJson, getResponse) || !getResponse->GetMember("address", address))
			{
				if (localServerRpc->AllotAddress(address))
				{
					Json::Writer setJson;
					setJson.AddMember("user_id", userId);
					setJson.AddMember("address", address);
					setJson.AddMember("service", config->Service);
					this->mRedisComponent->CallLua("user.set_address", setJson);
				}
			}
		}
		requestData->set_user_id(userId);
		requestData->set_method_id(config->InterfaceId);
		requestData->mutable_data()->PackFrom(request->data());
		std::shared_ptr<com::Rpc::Response> responseData = localServerRpc->StartCall(address, requestData);
		if (responseData != nullptr)
		{
			response->mutable_data()->PackFrom(responseData->data());
			return (XCode)responseData->code();
		}
		return XCode::NetWorkError;
	}
}