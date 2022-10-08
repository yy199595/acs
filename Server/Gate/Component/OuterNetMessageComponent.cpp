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
#include"Service/OuterService.h"
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

	XCode OuterNetMessageComponent::OnRequest(long long userId, std::shared_ptr<Rpc::Data> message)
    {
        std::string method, service;
        LOG_RPC_CHECK_ARGS(message->GetMethod(service, method));
        Service *localServerRpc = this->GetApp()->GetService(service);
        if (localServerRpc == nullptr)
        {
            return XCode::CallServiceNotFound;
        }
        const RpcServiceConfig &rpcServiceConfig = localServerRpc->GetServiceConfig();
        const RpcMethodConfig *config = rpcServiceConfig.GetConfig(method);
        if (config == nullptr || config->Type != "Client")
        {
            return XCode::NotFoundRpcConfig;
        }

        std::string address;
        if (!localServerRpc->GetHost(userId, address))
        {
            localServerRpc->GetHost(address);
            localServerRpc->AddHost(address, userId);
        }
        if (message->GetHead().Has("rpc"))
        {
            std::shared_ptr<ClientRpcTask> clientRpcTask
                = std::make_shared<ClientRpcTask>(*message, this, 0);

            message->GetHead().Remove("rpc");
            message->GetHead().Add("rpc", clientRpcTask->GetRpcId());
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
        return XCode::Successful;
    }


    void OuterNetMessageComponent::Auth(const std::string &address, const std::string &token)
    {

    }

	XCode OuterNetMessageComponent::OnResponse(const std::string & address, std::shared_ptr<Rpc::Data> message)
	{
        LOG_RPC_CHECK_ARGS(message->GetHead().Has("rpc"));
        if(message->GetCode(XCode::Failure) == XCode::NetActiveShutdown)
        {
            this->mOutNetComponent->StartClose(address);
            return XCode::NetActiveShutdown;
        }

        message->SetType(Tcp::Type::Response);
		if (!this->mOutNetComponent->SendData(address, message))
		{
            CONSOLE_LOG_ERROR("send message to client " << address << " error");
			return XCode::NetWorkError;
		}
        CONSOLE_LOG_DEBUG("send message to client " << address << " successful");
		return XCode::Successful;
	}
}