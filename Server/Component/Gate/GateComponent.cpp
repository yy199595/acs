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
		LOG_CHECK_RET_FALSE(this->mGateService = this->GetComponent<GateService>());
		LOG_CHECK_RET_FALSE(this->mRpcComponent = this->GetComponent<RpcComponent>());
		LOG_CHECK_RET_FALSE(this->mGateClientComponent = this->GetComponent<GateClientComponent>());
		return true;
	}

	void GateComponent::OnConnect(long long sockId)
	{
		std::shared_ptr<RequestTaskQueueSource> taskQueueSource =
			std::make_shared<RequestTaskQueueSource>();

	}

	XCode GateComponent::OnRequest(std::shared_ptr<c2s::Rpc_Request> request)
	{
		const std::string &address = request->address();
		const RpcConfig & rpcConfig = this->GetApp()->GetRpcConfig();
		const ProtoConfig * config = rpcConfig.GetProtocolConfig(request->method_name());
		if (config == nullptr)
		{
			this->mGateClientComponent->StartClose(address);
			LOG_ERROR("call function " << request->method_name() << " not find");
			return XCode::NotFoundRpcConfig;
		}

		return XCode::Successful;
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