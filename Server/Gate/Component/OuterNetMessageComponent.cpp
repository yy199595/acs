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
#include"Component/RedisDataComponent.h"
#include"Component/ProtoComponent.h"
#include"Component/UnitMgrComponent.h"
#include"Component/UnitLocationComponent.h"
namespace Sentry
{

	bool OuterNetMessageComponent::LateAwake()
	{
		this->mTaskComponent = this->GetApp()->GetTaskComponent();
		this->mTimerComponent = this->GetApp()->GetTimerComponent();
        this->mUnitComponent = this->GetComponent<UnitMgrComponent>();
        this->mInnerMessageComponent = this->GetComponent<InnerNetMessageComponent>();
		LOG_CHECK_RET_FALSE(this->mOutNetComponent = this->GetComponent<OuterNetComponent>());
		return true;
	}

	XCode OuterNetMessageComponent::OnRequest(long long userId, std::shared_ptr<Rpc::Data> message)
    {
        std::string method, service;
        std::shared_ptr<Unit> player = this->mUnitComponent->Find(userId);
        if(player == nullptr)
        {
            return XCode::NotFindUser;
        }
        LOG_RPC_CHECK_ARGS(message->GetMethod(service, method));
        Service *targetService = this->GetApp()->GetService(service);
        if (targetService == nullptr)
        {
            CONSOLE_LOG_ERROR("userid=" << userId <<
                " call [" << service << "] not find");
            return XCode::CallServiceNotFound;
        }
        const RpcMethodConfig *config = targetService->GetMethodConfig(method);
        if (config == nullptr || config->Type != "Client")
        {
            CONSOLE_LOG_ERROR("userid=" << userId <<
                " call [" << service << "." << method << "] not permissions");
            return XCode::NotFoundRpcConfig;
        }

        UnitLocationComponent * locationComponent = player->GetComponent<UnitLocationComponent>();
        if(locationComponent == nullptr)
        {
        }
        std::string address;        // TODO
        if(!locationComponent->Get(service, address))
        {
            if(!targetService->GetLocation(address))
            {
                return XCode::CallServiceNotFound;
            }
            locationComponent->Add(service, address);
            Service * innerService = this->GetApp()->GetService("InnerService");
            if(innerService != nullptr)
            {
                innerService->Call(address, "OnUserJoin");
            }
        }
        message->GetHead().Add("id", userId);
        this->mInnerMessageComponent->Send(address, message);
        return XCode::Successful;
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