//
// Created by mac on 2021/11/28.
//

#include"GateComponent.h"
#include"App/App.h"
#include"NetWork/RpcGateClient.h"
#include"Global/RpcConfig.h"
#include"Task/RpcProxyTask.h"
#include"Component/Rpc/RpcComponent.h"
#include"GateClientComponent.h"
#include"Component/Gate/GateService.h"
#include"Component/Rpc/RpcClientComponent.h"
#include"Component/RpcService/LocalServerRpc.h"
#ifdef __DEBUG__
#include"Pool/MessagePool.h"
#include"google/protobuf/util/json_util.h"
#endif
namespace Sentry
{

	bool GateComponent::LateAwake()
	{
		this->mTaskComponent = this->GetApp()->GetTaskComponent();
		LOG_CHECK_RET_FALSE(this->mRpcClientComponent = this->GetComponent<RpcClientComponent>());
		LOG_CHECK_RET_FALSE(this->mGateClientComponent = this->GetComponent<GateClientComponent>());
		return true;
	}

	void GateComponent::OnConnect(const std::string &)
	{
		std::shared_ptr<RequestTaskQueueSource> taskQueueSource =
			std::make_shared<RequestTaskQueueSource>();

	}

	XCode GateComponent::OnRequest(std::shared_ptr<c2s::Rpc_Request> request)
	{
		const std::string &address = request->address();
		const RpcConfig & rpcConfig = this->GetApp()->GetRpcConfig();
		const ProtoConfig * config = rpcConfig.GetProtocolConfig(request->method_name());
		if (config == nullptr || config->Type != "Client")
		{
			LOG_ERROR("call function " << request->method_name() << " not find");
			return XCode::NotFoundRpcConfig;
		}

		long long userId = this->mGateClientComponent->GetUserId(address);
#ifdef __DEBUG__
		LOG_INFO("========== client request ==========");
		LOG_INFO("func = " << request->method_name());
		LOG_INFO("address = " << request->address());
		LOG_INFO("userid = " << userId);
		if(request->has_data())
		{
			std::string json;
			Helper::Proto::GetJson(request->data(), json);
			LOG_INFO("json = " << json);
		}
#endif
		LocalServerRpc * localServerRpc = this->GetComponent<LocalServerRpc>(config->Service);
		if(localServerRpc == nullptr)
		{
			return XCode::CallServiceNotFound;
		}
		std::shared_ptr<com::Rpc::Request> clientRequest =
			std::make_shared<com::Rpc::Request>();

		clientRequest->set_user_id(userId);
		clientRequest->set_rpc_id(request->rpc_id());
		clientRequest->set_address(request->address());
		clientRequest->mutable_data()->CopyFrom(request->data());

		std::string json1;
		Helper::Proto::GetJson(clientRequest->data(), json1);

		if(userId == 0) //未登录
		{
			if(!localServerRpc->IsStartService())
			{
				return XCode::CallServiceNotFound;
			}

			std::shared_ptr<c2s::Rpc::Response> clientResponse(new c2s::Rpc::Response());
			std::shared_ptr<com::Rpc::Response> response = localServerRpc->Invoke(config->Method, clientRequest);

			if(response != nullptr)
			{
				clientResponse->set_code(response->code());
				clientResponse->set_rpc_id(clientResponse->rpc_id());
				clientResponse->mutable_data()->CopyFrom(response->data());
				this->mGateClientComponent->SendToClient(address, clientResponse);
			}
			return XCode::Successful;
		}
		std::string serviceAddress;
		if(!localServerRpc->GetEntityAddress(userId,serviceAddress))
		{
			return XCode::CallServiceNotFound;
		}
		this->mRpcClientComponent->Send(serviceAddress, clientRequest);
		return XCode::Successful;
	}

	void GateComponent::OnNotLogin(std::shared_ptr<c2s::Rpc_Request> request)
	{

	}

	XCode GateComponent::OnResponse(const std::string & address, std::shared_ptr<c2s::Rpc_Response> response)
	{
#ifdef __DEBUG__
		LOG_WARN("**********[client response]**********");
		if (response->has_data())
		{
			std::string json;
			std::shared_ptr<Message> message = Helper::Proto::NewByData(response->data());
			if (message != nullptr && Helper::Proto::GetJson(message, json))
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
}