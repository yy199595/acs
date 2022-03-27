//
// Created by mac on 2021/11/28.
//

#include"GateComponent.h"
#include"App/App.h"
#include"Service/ServiceProxy.h"
#include"NetWork/RpcGateClient.h"
#include"Component/Rpc/RpcConfigComponent.h"
#include"Component/Scene/ServiceMgrComponent.h"
#include"Task/RpcProxyTask.h"
#include"Component/Rpc/RpcComponent.h"
#include"GateClientComponent.h"
#ifdef __DEBUG__
#include"Pool/MessagePool.h"
#include"google/protobuf/util/json_util.h"
#endif
namespace Sentry
{
	bool GateComponent::Awake()
	{
		this->mServiceComponent = nullptr;
		this->mRpcConfigComponent = nullptr;
		this->mGateClientComponent = nullptr;
		return true;
	}

	bool GateComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mRpcComponent = this->GetComponent<RpcComponent>());
		LOG_CHECK_RET_FALSE(this->mRpcConfigComponent = this->GetComponent<RpcConfigComponent>());
		LOG_CHECK_RET_FALSE(this->mServiceComponent = this->GetComponent<ServiceMgrComponent>());
		LOG_CHECK_RET_FALSE(this->mGateClientComponent = this->GetComponent<GateClientComponent>());
		return true;
	}

	XCode GateComponent::OnRequest(std::shared_ptr<c2s::Rpc_Request> request)
	{
		auto config = this->mRpcConfigComponent->GetProtocolConfig(request->method_name());
		if (config == nullptr)
		{
			LOG_ERROR("call function ", request->method_name(), " not find");
			return XCode::NotFoundRpcConfig;
		}

		//TODO
		auto serviceEntity = this->mServiceComponent->GetServiceProxy(config->Service);

		if (!config->Request.empty())
		{
			if (!request->has_data()) //没有正确的消息体
			{
				return XCode::CallArgsError;
			}
			this->mProtoName.clear();
			if (Any::ParseAnyTypeUrl(request->data().type_url(), &mProtoName))
			{
				if (this->mProtoName != config->Request) //请求的消息不正确
				{
					return XCode::CallArgsError;
				}
			}
		}

		// 分配地址
		auto requestMessage = serviceEntity->NewRequest(request->method_name());
		std::shared_ptr<RpcProxyTask> proxyTask(new RpcProxyTask());
		if (request->has_data())
		{
			requestMessage->mutable_data()->CopyFrom(request->data());
		}
		requestMessage->set_rpc_id(proxyTask->GetRpcId());
		proxyTask->InitProxyTask(request->rpc_id(), request->sock_id(), this, this->mRpcComponent);
		return XCode::Successful;
	}

	XCode GateComponent::OnResponse(long long sockId, std::shared_ptr<c2s::Rpc_Response> response)
	{
#ifdef __DEBUG__
		LOG_WARN("**********[client response]**********");
		if (response->has_data())
		{
			std::string json;
			std::shared_ptr<Message> message = Helper::Proto::NewByData(response->data());
			if (message != nullptr && Helper::Proto::GetJson(message, json))
			{
				LOG_WARN("json = ", json);
			}
		}
		LOG_WARN("*****************************************");
#endif
		if (this->mGateClientComponent->SendToClient(sockId, response))
		{
			return XCode::NetWorkError;
		}
		return XCode::Successful;
	}
}