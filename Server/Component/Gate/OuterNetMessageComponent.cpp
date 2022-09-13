//
// Created by mac on 2021/11/28.
//

#include"OuterNetMessageComponent.h"
#include"App/App.h"
#include"NetWork/OuterNetClient.h"
#include"Global/ServiceConfig.h"
#include"Component/Rpc/InnerNetMessageComponent.h"
#include"OuterNetComponent.h"
#include"Component/Rpc/InnerNetComponent.h"
#include"Component/RpcService/LocalService.h"
#include"GateService.h"
#include"Component/User/UserSyncComponent.h"
#include"Component/Redis/RedisDataComponent.h"
#include"Component/Scene/ProtoComponent.h"

namespace Sentry
{
    ClientRpcTask::ClientRpcTask(const c2s::rpc::request &request, OuterNetMessageComponent * component, int ms)
        : IRpcTask<com::rpc::response>(ms)
    {
        this->mTaskId = Guid::Create();
        this->mRpcId = request.rpc_id();
        this->mGateComponent = component;
        this->mAddress = request.address();
    }

    void ClientRpcTask::OnTimeout()
    {
        std::shared_ptr<com::rpc::response> response(new com::rpc::response());

        response->set_rpc_id(this->mRpcId);
        response->set_code((int)XCode::CallTimeout);
        this->mGateComponent->OnResponse(this->mAddress, response);
    }

    void ClientRpcTask::OnResponse(std::shared_ptr<com::rpc::response> response)
    {
        response->set_rpc_id(this->mRpcId);
        this->mGateComponent->OnResponse(this->mAddress, response);
    }
}

namespace Sentry
{

	bool OuterNetMessageComponent::LateAwake()
	{
		this->mTaskComponent = this->GetApp()->GetTaskComponent();
		this->mTimerComponent = this->GetApp()->GetTimerComponent();
        this->mInnerMessageComponent = this->GetComponent<InnerNetMessageComponent>();
		LOG_CHECK_RET_FALSE(this->mOutNetComponent = this->GetComponent<OuterNetComponent>());
		return true;
	}

	XCode OuterNetMessageComponent::OnRequest(std::shared_ptr<c2s::rpc::request> request)
	{
		std::string method, service;
        assert(this->GetApp()->IsMainThread());
        if(!RpcServiceConfig::ParseFunName(request->method_name(), service, method))
		{
			LOG_ERROR("call function " << request->method_name() << " not find");
			return XCode::NotFoundRpcConfig;
		}
		Service* localServerRpc = this->GetApp()->GetService(service);
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

        std::shared_ptr<com::rpc::request> userRequest(new com::rpc::request());

        userRequest->set_func(config->FullName);
        userRequest->set_rpc_id(request->rpc_id());
        userRequest->set_address(request->address());
        userRequest->set_type(com::rpc_msg_type_proto);
        userRequest->mutable_data()->CopyFrom(request->data());

        if (!this->mOutNetComponent->GetUserId(address, userId) && config->IsAuth) //没有验证
        {
            GateService * gateService = localServerRpc->Cast<GateService>();
            if(gateService == nullptr)
            {
                return XCode::CallServiceNotFound;
            }
            this->mTaskComponent->Start([gateService, userRequest, config, this]()
            {
                const std::string & userAddress = userRequest->address();
                std::shared_ptr<com::rpc::response> response(new com::rpc::response());
                XCode code = gateService->Invoke(config->Method, userRequest, response);
                if(code != XCode::Successful)
                {
                    this->mOutNetComponent->StartClose(userAddress);
                    return;
                }
                response->set_code((int)code);
                response->set_rpc_id(userRequest->rpc_id());
                this->OnResponse(userAddress, response);
            });
        }
        else
        {
            std::string targetAddress;
            userRequest->set_user_id(userId);
            if(!localServerRpc->GetHost(userId, targetAddress))
            {
                localServerRpc->GetHost(targetAddress);
                localServerRpc->AddHost(targetAddress, userId);
            }
            std::shared_ptr<ClientRpcTask> clientRpcTask
                = std::make_shared<ClientRpcTask>(*request, this, 0);

            userRequest->set_rpc_id(clientRpcTask->GetRpcId());
            this->mInnerMessageComponent->AddTask(clientRpcTask);
            localServerRpc->SendRequest(targetAddress, userRequest);
            //CONSOLE_LOG_ERROR("send message to [" << targetAddress << "]");
        }
		return XCode::Successful;
	}

	XCode OuterNetMessageComponent::OnResponse(const std::string & address, std::shared_ptr<com::rpc::response> response)
	{
        assert(this->GetApp()->IsMainThread());
        if(response->code() == (int)XCode::NetActiveShutdown)
        {
            this->mOutNetComponent->StartClose(address);
            return XCode::NetActiveShutdown;
        }
        std::shared_ptr<c2s::rpc::response> clientResponse(new c2s::rpc::response());

        clientResponse->set_code(response->code());
        clientResponse->set_rpc_id(response->rpc_id());
        clientResponse->set_error_str(response->error_str());
        clientResponse->mutable_data()->CopyFrom(response->data());
		if (!this->mOutNetComponent->SendToClient(address, clientResponse))
		{
            CONSOLE_LOG_ERROR("send message to client " << address << " error");
			return XCode::NetWorkError;
		}
		return XCode::Successful;
	}
}