//
// Created by mac on 2021/11/28.
//

#include"GateComponent.h"
#include"App/App.h"
#include"NetWork/GateClientContext.h"
#include"Global/ServiceConfig.h"
#include"Component/Rpc/ServiceRpcComponent.h"
#include"GateClientComponent.h"
#include"Component/Rpc/RpcClientComponent.h"
#include"Component/RpcService/LocalServiceComponent.h"
#include"GateService.h"
#include"Component/User/UserSyncComponent.h"
#include"Component/Redis/MainRedisComponent.h"
#include"Component/Scene/MessageComponent.h"

namespace Sentry
{
    ClientRpcTask::ClientRpcTask(const c2s::Rpc::Request &request, GateClientComponent * component)
    {
        this->mTaskId = Guid::Create();
        this->mClientComponent = component;
        this->mAddress = request.address();
        this->mResponse = std::make_shared<c2s::Rpc::Response>();
        this->mResponse->set_rpc_id(request.rpc_id());
        this->mResponse->set_code((int)XCode::CallTimeout);
    }

    void ClientRpcTask::OnResponse(std::shared_ptr<Rpc_Response> response)
    {
        if(response != nullptr)
        {
            this->mResponse->set_code(response->code());
            this->mResponse->set_error_str(response->error_str());
            if (response->code() == (int) XCode::Successful && response->has_data())
            {
                this->mResponse->mutable_data()->CopyFrom(response->data());
            }
        }
        this->mClientComponent->SendToClient(this->mAddress, this->mResponse);
    }
}

namespace Sentry
{

	bool GateComponent::LateAwake()
	{
		this->mTaskComponent = this->GetApp()->GetTaskComponent();
		this->mTimerComponent = this->GetApp()->GetTimerComponent();
		this->mMsgComponent = this->GetComponent<MessageComponent>();
		this->mUserSyncComponent = this->GetComponent<UserSyncComponent>();
        this->mServiceRpcComponent = this->GetComponent<ServiceRpcComponent>();
		LOG_CHECK_RET_FALSE(this->mGateClientComponent = this->GetComponent<GateClientComponent>());
		return true;
	}

	XCode GateComponent::OnRequest(std::shared_ptr<c2s::Rpc_Request> request)
	{
		std::string method, service;
		if(!RpcServiceConfig::ParseFunName(request->method_name(), service, method))
		{
			LOG_ERROR("call function " << request->method_name() << " not find");
			return XCode::NotFoundRpcConfig;
		}
		ServiceComponent* localServerRpc = this->GetApp()->GetService(service);
		if(localServerRpc == nullptr)
		{
			return XCode::CallServiceNotFound;
		}
		const RpcServiceConfig & rpcServiceConfig = localServerRpc->GetServiceConfig();
		const RpcInterfaceConfig* config = rpcServiceConfig.GetConfig(method);
		if(config == nullptr || config->Type != "Client")
		{
			return XCode::NotFoundRpcConfig;
		}

		if (!config->Request.empty())
		{
            if(!request->has_data())
            {
                return XCode::CallArgsError;
            }
			std::string fullName;
            const std::string & url = request->data().type_url();
			if (!Any::ParseAnyTypeUrl(url, &fullName) || fullName != config->Request)
			{
				return XCode::CallArgsError;
			}
		}
        long long userId = 0;
        const std::string & address = request->address();

        std::shared_ptr<com::Rpc::Request> userRequest(new com::Rpc::Request());

        userRequest->set_func(config->FullName);
        userRequest->set_rpc_id(request->rpc_id());
        userRequest->set_address(request->address());
        userRequest->mutable_data()->CopyFrom(request->data());

        if (!this->mGateClientComponent->GetUserId(address, userId) && config->IsAuth) //没有验证
        {
            GateService * gateService = localServerRpc->Cast<GateService>();
            if(gateService == nullptr)
            {
                return XCode::CallServiceNotFound;
            }
            this->mTaskComponent->Start([gateService, userRequest, config, this]()
            {
                const std::string & userAddress = userRequest->address();
                std::shared_ptr<com::Rpc::Response> response(new com::Rpc::Response());
                XCode code = gateService->Invoke(config->Method, userRequest, response);
                if(code != XCode::Successful)
                {
                    this->mGateClientComponent->StartClose(userAddress);
                    return;
                }
                std::shared_ptr<c2s::Rpc::Response> userResponse(new c2s::Rpc::Response());

                userResponse->set_code((int)code);
                userResponse->set_rpc_id(userRequest->rpc_id());
                userResponse->mutable_data()->CopyFrom(response->data());
                this->mGateClientComponent->SendToClient(userAddress, userResponse);
            });
        }
        else
        {
            std::string targetAddress;
            userRequest->set_user_id(userId);
            AddressProxy & addressProxy = localServerRpc->GetAddressProxy();
            if(!addressProxy.GetAddress(userId, targetAddress))
            {
                addressProxy.GetAddress(targetAddress);
                addressProxy.AddUserAddress(userId, targetAddress);
            }
            std::shared_ptr<ClientRpcTask> clientRpcTask
                = std::make_shared<ClientRpcTask>(*request, this->mGateClientComponent);

            userRequest->set_rpc_id(clientRpcTask->GetRpcId());
            this->mServiceRpcComponent->AddTask(clientRpcTask);
            localServerRpc->SendRequest(targetAddress, userRequest);
        }
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
}