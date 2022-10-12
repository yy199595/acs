//
// Created by mac on 2021/11/28.
//

#include"OuterNetMessageComponent.h"
#include"Client/OuterNetClient.h"
#include"Config/ServiceConfig.h"
#include"Component/InnerNetMessageComponent.h"
#include"OuterNetComponent.h"
#include"Component/InnerNetComponent.h"
#include"Service/LocalService.h"
#include"Service/InnerService.h"
#include"Component/RedisDataComponent.h"
#include"Component/ProtoComponent.h"

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

        std::string address;
        if(!targetService->GetLocation(userId, address))
        {
            if(!targetService->AllotLocation(userId, address))
            {
                return XCode::Failure;
            }
            s2s::location::sync request;
            request.set_name(service);
            request.set_user_id(userId);
            InnerService * service = this->GetComponent<InnerService>();
            if(service->Send(address, "OnUserJoin", request) != XCode::Successful)
            {
                return XCode::NetWorkError;
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