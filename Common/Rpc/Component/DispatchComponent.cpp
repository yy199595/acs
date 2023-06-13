#include"DispatchComponent.h"

#include"Async/Component/CoroutineComponent.h"
#include"Server/Config/ServiceConfig.h"
#include"InnerNetComponent.h"
#include"Timer/Timer/ElapsedTimer.h"
#include"Server/Config/CodeConfig.h"
#ifdef __DEBUG__
#include"Util/String/StringHelper.h"
#include"Proto/Component/ProtoComponent.h"
#endif
#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Rpc/Client/Message.h"
#include"Rpc/Service/RpcService.h"
#include"Gate/Component/OuterNetComponent.h"
#include"Router/Component/RouterComponent.h"
#ifdef __RPC_MESSAGE__
#include"Proto/Include/Message.h"
#endif

namespace Tendo
{
	DispatchComponent::DispatchComponent()
	{
		this->mWaitCount = 0;
		this->mRouterComponent = nullptr;
		this->mTaskComponent = nullptr;
		this->mOuterComponent = nullptr;
		this->mTimerComponent = nullptr;
	}

	bool DispatchComponent::LateAwake()
	{
        this->mTimerComponent = this->mApp->GetTimer();
		this->mTaskComponent = this->mApp->GetCoroutine();
		this->mRouterComponent = this->GetComponent<RouterComponent>();
		this->mOuterComponent = this->GetComponent<OuterNetComponent>();
		return true;
	}

	int DispatchComponent::OnRequestMessage(const std::shared_ptr<Msg::Packet> & message)
	{
		const std::string & fullName = message->ConstHead().GetStr("func");
        const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
        if(methodConfig == nullptr)
        {
            return XCode::CallFunctionNotExist;
        }
        if(this->mApp->GetComponentByName(methodConfig->Service) == nullptr)
        {
            LOG_ERROR("call service not exist : [" << methodConfig->Service << "]");
            return XCode::CallServiceNotFound;
        }
        if (!methodConfig->IsAsync)
        {
            this->Invoke(methodConfig, message);
            return XCode::Successful;
        }
        this->mTaskComponent->Start(&DispatchComponent::Invoke, this, methodConfig, message);
		return XCode::Successful;
	}

    void DispatchComponent::Invoke(const RpcMethodConfig *config, const std::shared_ptr<Msg::Packet>& message)
	{
		this->AddWaitCount(config->Service);
		const std::string & service = config->Service;
		RpcService* logicService = this->GetComponent<RpcService>(service);
		if (logicService == nullptr)
		{
			this->SubWaitCount(service);
			LOG_ERROR("call [" << config->FullName << "] server not found");
			return;
		}
		long long timerId = 0;
		if (config->Timeout > 0)
		{
			timerId = this->mTimerComponent->DelayCall(config->Timeout,
					[message, config, this]()
					{
						this->SubWaitCount(config->Service);
						LOG_ERROR("call [" << config->FullName << "] code = call time out");
						this->mRouterComponent->Send(message->From(), XCode::CallTimeout, message);
					}
			);
		}

		this->mWaitCount++;
		const int code = logicService->Invoke(config->Method, message);
		{
			this->SubWaitCount(service);
		}

		this->mWaitCount--;
		if (timerId > 0 && !this->mTimerComponent->CancelTimer(timerId))
		{
			LOG_ERROR("call [" << config->FullName << "] time out not return");
			return;
		}
		const std::string& address = message->From();
		this->mRouterComponent->Send(address, code, message);
	}

	bool DispatchComponent::EncodeJson(const std::shared_ptr<Msg::Packet>& message, std::string& json) const
	{
		switch(message->GetProto())
		{
		case Msg::Porto::Json:
			json = message->GetBody();
			return true;
		case Msg::Porto::Protobuf:
			{
				std::string func;
				message->ConstHead().Get("func", func);
				const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(func);
				if(methodConfig == nullptr)
				{
					return false;
				}
				std::shared_ptr<Message> data;
				const std::string & pb = message->GetType() ==
					Msg::Type::Response ? methodConfig->Response : methodConfig->Request;
				if(!pb.empty() && this->mApp->GetProto()->New(pb, data))
				{
					return pb_json::MessageToJsonString(*data, &json).ok();
				}
			}
			break;
		}
		return false;
	}


    int DispatchComponent::OnMessage(const std::shared_ptr<Msg::Packet>& message)
    {
        switch (message->GetType())
        {
            case Msg::Type::Request:
                return this->OnRequestMessage(message);
            case Msg::Type::Response:
                return this->OnResponseMessage(message);
            case Msg::Type::Client:
                return this->OnForwardMessage(message);
            case Msg::Type::Broadcast:
                return this->OnBroadcastMessage(message);
			case Msg::Type::SubPublish:
				return this->OnPublishMessage(message);
			case Msg::Type::Forward:
				// TODO 中转
				break;
        }
		LOG_ERROR("unknown message type : " << message->GetType());
        return XCode::Successful;
    }

	int DispatchComponent::OnPublishMessage(const std::shared_ptr<Msg::Packet>& message)
	{
		return XCode::Successful;
	}

	int DispatchComponent::OnForwardMessage(const std::shared_ptr<Msg::Packet>& message)
	{
		if (this->mOuterComponent == nullptr)
		{
			return XCode::NetWorkError;
		}
		long long userId = 0;
		if (!message->GetHead().Get("id", userId))
		{
			return XCode::CallArgsError;
		}
		message->GetHead().Remove("id");
		message->SetType(Msg::Type::Request);
		if(!this->mOuterComponent->Send(userId, message))
		{
			return XCode::NotFindUser;
		}
		return XCode::Successful;
	}

	int DispatchComponent::OnResponseMessage(const std::shared_ptr<Msg::Packet>& message)
    {
        if (this->mOuterComponent != nullptr)
        {
            std::string address;
            // 网关转发过来的消息 必须带client字段
            if (message->GetHead().Get("cli", address))
            {
                message->GetHead().Remove("cli");
                this->mOuterComponent->Send(address, message);
                return XCode::Successful;
            }
        }
        int rpcId = 0;
        if (!message->GetHead().Get("rpc", rpcId))
        {
            return XCode::CallArgsError;
        }
        this->OnResponse(rpcId, message);
        return XCode::Successful;
    }

	int DispatchComponent::OnBroadcastMessage(const std::shared_ptr<Msg::Packet>& message)
	{
        if(this->mOuterComponent == nullptr)
        {
            return XCode::Failure;
        }
        message->SetType(Msg::Type::Request);
        this->mOuterComponent->Broadcast(message);
		return XCode::Successful;
	}

	void DispatchComponent::AddWaitCount(const std::string& name)
	{
		auto iter = this->mWaitCounts.find(name);
		if(iter == this->mWaitCounts.end())
		{
			this->mWaitCounts.emplace(name, 1);
			return;
		}
		iter->second++;
	}

	void DispatchComponent::SubWaitCount(const std::string& name)
	{
		auto iter = this->mWaitCounts.find(name);
		if(iter != this->mWaitCounts.end())
		{
			iter->second--;
		}
	}
}// namespace Sentry
