//
// Created by mac on 2021/11/28.
//

#include"OuterNetMessageComponent.h"
#include"App/App.h"
#include"Client/OuterNetClient.h"
#include"Config/ServiceConfig.h"
#include"Component/InnerNetMessageComponent.h"
#include"OuterNetComponent.h"
#include"Component/InnerNetComponent.h"
#include"Service/LocalService.h"
#include"Service/GateService.h"
#include"Component/UserSyncComponent.h"
#include"Component/RedisDataComponent.h"
#include"Component/ProtoComponent.h"

namespace Sentry
{
    ClientRpcTask::ClientRpcTask(Rpc::Data &request, OuterNetMessageComponent * component, int ms)
        : IRpcTask<Rpc::Data>(ms)
    {
        this->mTaskId = Guid::Create();
        this->mGateComponent = component;
        request.GetHead().Get("rpc", this->mRpcId);
        request.GetHead().Get("address", this->mAddress);
    }

    void ClientRpcTask::OnTimeout()
    {
        std::shared_ptr<Rpc::Data> message(new Rpc::Data());

        message->SetType(Tcp::Type::Response);
        message->SetProto(Tcp::Porto::Protobuf);
        message->GetHead().Add("rpc", this->mRpcId);
        message->GetHead().Add("code", (int)XCode::CallTimeout);
        this->mGateComponent->OnResponse(this->mAddress, message);
    }

    void ClientRpcTask::OnResponse(std::shared_ptr<Rpc::Data> response)
    {
        response->GetHead().Remove("rpc");
        response->GetHead().Add("rpc", this->mRpcId);
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

	XCode OuterNetMessageComponent::OnRequest(const std::string & address, std::shared_ptr<Rpc::Data> message)
    {
        std::string method, service;
        LOG_RPC_CHECK_ARGS(message->GetMethod(service, method));
        Service *localServerRpc = this->GetApp()->GetService(service);
        if (localServerRpc == nullptr)
        {
            return XCode::CallServiceNotFound;
        }
        const RpcServiceConfig &rpcServiceConfig = localServerRpc->GetServiceConfig();
        const RpcInterfaceConfig *config = rpcServiceConfig.GetConfig(method);
        if (config == nullptr || config->Type != "Client")
        {
            return XCode::NotFoundRpcConfig;
        }
        long long userId = 0;
        LOG_RPC_CHECK_ARGS(!config->Request.empty() && message->GetBody()->empty());
        if (!this->mOutNetComponent->GetUserId(address, userId) && config->IsAuth) //没有验证
        {
            GateService *gateService = localServerRpc->Cast<GateService>();
            if (gateService == nullptr)
            {
                return XCode::CallServiceNotFound;
            }
            this->mTaskComponent->Start([gateService, message, config, this, address]() {
                XCode code = gateService->Invoke(config->Method, message);
                if (code != XCode::Successful)
                {
                    this->mOutNetComponent->StartClose(address);
                    return;
                }
                this->mOutNetComponent->SendData(address, message);
            });
        }
        else
        {
            std::string targetAddress;
            if (!localServerRpc->GetHost(userId, targetAddress))
            {
                localServerRpc->GetHost(targetAddress);
                localServerRpc->AddHost(targetAddress, userId);
            }
            long long rpcId = 0;
            if (message->GetHead().Get("rpc", rpcId))
            {
                std::shared_ptr<ClientRpcTask> clientRpcTask
                    = std::make_shared<ClientRpcTask>(*message, this, 0);
                if (!this->mInnerMessageComponent->Send(address, message))
                {
                    return XCode::SendMessageFail;
                }
                this->mInnerMessageComponent->AddTask(clientRpcTask);
            }
            else
            {
                if (!this->mInnerMessageComponent->Send(address, message))
                {
                    return XCode::SendMessageFail;
                }
            }
        }
        return XCode::Successful;
    }

	XCode OuterNetMessageComponent::OnResponse(const std::string & address, std::shared_ptr<Rpc::Data> response)
	{
        assert(this->GetApp()->IsMainThread());
        LOG_RPC_CHECK_ARGS(response->GetHead().Has("rpc"));
        if(response->GetCode(XCode::Failure) == XCode::NetActiveShutdown)
        {
            this->mOutNetComponent->StartClose(address);
            return XCode::NetActiveShutdown;
        }

		if (!this->mOutNetComponent->SendData(address, response))
		{
            CONSOLE_LOG_ERROR("send message to client " << address << " error");
			return XCode::NetWorkError;
		}
		return XCode::Successful;
	}
}