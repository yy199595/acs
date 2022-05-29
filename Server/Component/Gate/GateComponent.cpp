//
// Created by mac on 2021/11/28.
//

#include"GateComponent.h"
#include"App/App.h"
#include"NetWork/GateClientContext.h"
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
#include"GateProxyComponent.h"
#include"Component/Redis/MainRedisComponent.h"
#include"Component/User/UserSyncComponent.h"
namespace Sentry
{

	bool GateComponent::LateAwake()
	{
		this->mTaskComponent = this->GetApp()->GetTaskComponent();
		this->mTimerComponent = this->GetApp()->GetTimerComponent();
		this->mUserSyncComponent = this->GetComponent<UserSyncComponent>();
		LOG_CHECK_RET_FALSE(this->mGateClientComponent = this->GetComponent<GateClientComponent>());
		return true;
	}

	XCode GateComponent::OnRequest(std::shared_ptr<c2s::Rpc_Request> request)
	{
		const ServiceConfig& rpcConfig = this->GetApp()->GetServiceConfig();
		const RpcInterfaceConfig* config = rpcConfig.GetInterfaceConfig(request->method_name());
		if (config == nullptr || config->Type != "Client")
		{
			LOG_ERROR("call function " << request->method_name() << " not find");
			return XCode::NotFoundRpcConfig;
		}

		if (!config->Request.empty())
		{
			std::string fullName;
			if (!request->has_data() || !Any::ParseAnyTypeUrl(
				request->data().type_url(), &fullName) || fullName != config->Request)
			{
				return XCode::CallArgsError;
			}
		}

		std::shared_ptr<com::Rpc::Request> userRequest =
			std::make_shared<com::Rpc::Request>();
		userRequest->set_rpc_id(request->rpc_id());
		userRequest->set_address(request->address());
		userRequest->set_method_id(config->InterfaceId);
		userRequest->mutable_data()->CopyFrom(request->data());
		this->mTaskComponent->Start(&GateComponent::OnUserRequest, this, config, userRequest);
		return XCode::Successful;
	}

	XCode GateComponent::OnResponse(const std::string & address, std::shared_ptr<c2s::Rpc_Response> response)
	{
		if (this->mGateClientComponent->SendToClient(address, response))
		{
			return XCode::NetWorkError;
		}
		return XCode::Successful;
	}

	void GateComponent::OnUserRequest(const RpcInterfaceConfig * config, std::shared_ptr<com::Rpc::Request> request)
	{
#if __RPC_DEBUG_LOG__
		std::string json;
		long long userId = 0;
		LOG_DEBUG("========== client request ==========");
		LOG_DEBUG("func = " << config->FullName);
		LOG_DEBUG("rpc = " << request->rpc_id());
		if (this->mGateClientComponent->GetUserId(request->address(), userId))
		{
			LOG_DEBUG("userId = " << userId);
		}
		if (request->has_data() && Proto::GetJson(request->data(), json))
		{
			LOG_DEBUG("json = " << json);
		}
		LOG_DEBUG("=====================================");
#endif
		std::shared_ptr<c2s::Rpc::Response> response =
			std::make_shared<c2s::Rpc::Response>();
		response->set_rpc_id(request->rpc_id());
		XCode code = this->HandlerRequest(config, request, response);
		if (code != XCode::Successful)
		{
			this->mGateClientComponent->StartClose(request->address());
			return;
		}
#if __RPC_DEBUG_LOG__
		LOG_WARN("********** client response**********");
		LOG_DEBUG("func = " << config->FullName);
		if (response->has_data() && Helper::Proto::GetJson(response->data(), json))
		{
			LOG_WARN("json = " << json);
		}
		LOG_WARN("*****************************************");
#endif
		this->mGateClientComponent->SendToClient(request->address(), response);
	}

	XCode GateComponent::HandlerRequest(const RpcInterfaceConfig * config,
		std::shared_ptr<com::Rpc::Request> rpcRequest, std::shared_ptr<c2s::Rpc::Response> response)
	{
		long long userId = 0;
		if (!this->mGateClientComponent->GetUserId(rpcRequest->address(), userId)) //没有验证
		{
			GateService* gateService = this->GetComponent<GateService>(config->Service);
			if (gateService == nullptr || !gateService->IsStartService())
			{
				return XCode::CallServiceNotFound;
			}
			std::shared_ptr<com::Rpc::Response> rpcResponse = std::make_shared<com::Rpc::Response>();
			XCode code = gateService->Invoke(config->Method, rpcRequest, rpcResponse);
			if (code == XCode::Successful)
			{
				response->mutable_data()->CopyFrom(rpcResponse->data());
				return XCode::Successful;
			}
			LOG_ERROR(rpcRequest->address() << " auth failure");
			return XCode::NetActiveShutdown;
		}
		std::string address;
		LocalRpcService* localServerRpc = this->GetComponent<LocalRpcService>(config->Service);
		AddressProxy & serviceAddressProxy = localServerRpc->GetAddressProxy();
		if (!serviceAddressProxy.GetUserAddress(userId, address))
		{
			address = this->mUserSyncComponent->GetAddress(userId, config->Service);
			if (serviceAddressProxy.HasAddress(address) || serviceAddressProxy.GetAddress(address))
			{
				localServerRpc->GetAddressProxy().AddUserAddress(userId, address);
				this->mUserSyncComponent->SetAddress(userId, config->Service, address);
				LOG_DEBUG(userId << "  " << config->Service << " allot address = " << address);
			}
		}
		rpcRequest->set_user_id(userId);
		std::shared_ptr<Message> rpcResponse = config->Response.empty()
											   ? nullptr : Helper::Proto::New(config->Response);

		XCode code = localServerRpc->Call(address, rpcRequest, rpcResponse);

		response->set_code((int)code);
		if (code == XCode::Successful && rpcResponse != nullptr)
		{
			response->mutable_data()->PackFrom(*rpcResponse);
		}
		return XCode::Successful;
	}
}