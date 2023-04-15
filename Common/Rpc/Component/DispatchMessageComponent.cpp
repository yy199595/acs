#include"DispatchMessageComponent.h"

#include"Async/Component/AsyncMgrComponent.h"
#include"Rpc/Lua/LuaServiceMethod.h"
#include"Server/Config/ServiceConfig.h"
#include"InnerNetComponent.h"
#include"Timer/Timer/ElapsedTimer.h"
#include"Rpc/Service/PhysicalRpcService.h"
#include"Server/Config/CodeConfig.h"
#ifdef __DEBUG__
#include"Util/String/StringHelper.h"
#include"Proto/Component/ProtoComponent.h"
#endif
#include"Entity/Unit/App.h"
#include"Rpc/Client/Message.h"
#include"Rpc/Component/InnerRpcComponent.h"
#include"Gate/Component/OuterNetComponent.h"
#include "google/protobuf/util/json_util.h"

namespace Tendo
{
	DispatchMessageComponent::DispatchMessageComponent()
	{
		this->mWaitCount = 0;
		this->mNetComponent = nullptr;
		this->mTaskComponent = nullptr;
		this->mOuterComponent = nullptr;
		this->mTimerComponent = nullptr;
	}

	bool DispatchMessageComponent::LateAwake()
	{
        this->mTimerComponent = this->mApp->GetTimerComponent();
		this->mNetComponent = this->GetComponent<InnerRpcComponent>();
		this->mOuterComponent = this->GetComponent<OuterNetComponent>();
		LOG_CHECK_RET_FALSE(this->mTaskComponent = this->GetComponent<AsyncMgrComponent>());
		return true;
	}

	int DispatchMessageComponent::HandlerRequest(const std::shared_ptr<Rpc::Packet> & message)
	{
        //const Rpc::Head & head = message->ConstHead();
		const std::string & fullName = message->ConstHead().GetStr("func");
        const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
        if(methodConfig == nullptr)
        {
            return XCode::CallFunctionNotExist;
        }
        if(this->mApp->GetService(methodConfig->Service) == nullptr)
        {
            LOG_ERROR("call service not exist : [" << methodConfig->Service << "]");
            return XCode::CallServiceNotFound;
        }
        if (!methodConfig->IsAsync)
        {
            this->Invoke(methodConfig, message);
            return XCode::Successful;
        }
        this->mTaskComponent->Start(&DispatchMessageComponent::Invoke, this, methodConfig, message);
		return XCode::Successful;
	}

    void DispatchMessageComponent::Invoke(const RpcMethodConfig *config, const std::shared_ptr<Rpc::Packet>& message)
	{
		RpcService* logicService = this->mApp->GetService(config->Service);
		if (logicService == nullptr || !logicService->IsStartService())
		{
			LOG_ERROR("call [" << config->FullName << "] server not found");
			return;
		}
		long long timerId = 0;
		if (config->Timeout > 0)
		{
			timerId = this->mTimerComponent->DelayCall(config->Timeout,
					[message, config, this]()
					{
						LOG_ERROR("call [" << config->FullName << "] code = call time out");
						this->mNetComponent->Send(message->From(), XCode::CallTimeout, message);
					}
			);
		}
#ifdef __RPC_MESSAGE__
		std::string json("{}");
		std::string serverName = message->From();
		ProtoComponent* component = this->GetComponent<ProtoComponent>();
		if (!config->Request.empty())
		{
			switch(message->GetProto())
			{
				case Msg::Porto::Json:
					json = message->GetBody();
					break;
				case Msg::Porto::Protobuf:
				{
					std::shared_ptr<Message> request;
					if (component->New(config->Request, request))
					{
						if (request->ParseFromString(message->GetBody()))
						{
							json.clear();
							google::protobuf::util::MessageToJsonString(*request, &json);
						}
					}
				}
			}
		}
		CONSOLE_LOG_DEBUG(serverName << " call func = ["
									 << config->FullName << "] request = " << Helper::Str::FormatJson(json));
#endif
		this->mWaitCount++;
		int code = logicService->Invoke(config->Method, message);
#ifdef __RPC_MESSAGE__
		json.assign("{}");
		if (code == XCode::Successful && !config->Response.empty())
		{
			switch(message->GetProto())
			{
				case Msg::Porto::Json:
					json = message->GetBody();
					break;
				case Msg::Porto::Protobuf:
				{
					std::shared_ptr<Message> response;
					if (component->New(config->Response, response))
					{
						if (response->ParseFromString(message->GetBody()))
						{
							json.clear();
							google::protobuf::util::MessageToJsonString(*response, &json);
						}
					}
					break;
				}
			}
		}
		if (code == XCode::Successful && message->GetHead().Has("rpc"))
		{
			CONSOLE_LOG_INFO(serverName << " call func = ["
										<< config->FullName << "] " << " response = " << Helper::Str::FormatJson(json));
		}
		else if(code != XCode::Successful)
		{
			const std::string& desc = CodeConfig::Inst()->GetDesc(code);
			CONSOLE_LOG_ERROR(serverName << " call func = [" << config->FullName << "] code = " << desc);
		}
		//LOG_INFO("call [" << config->FullName << "] use time = " << timer.GetMs() << "ms");
#endif
		this->mWaitCount--;
		if (timerId > 0 && !this->mTimerComponent->CancelTimer(timerId))
		{
			LOG_ERROR("call [" << config->FullName << "] time out not return");
			return;
		}
		const std::string& address = message->From();
		this->mNetComponent->Send(address, code, message);
	}

    int DispatchMessageComponent::OnMessage(const std::shared_ptr<Rpc::Packet>& message)
    {
        switch (message->GetType())
        {
            case Msg::Type::Request:
                return this->HandlerRequest(message);
            case Msg::Type::Response:
                return this->HandlerResponse(message);
            case Msg::Type::Forward:
                return this->HandlerForward(message);
            case Msg::Type::Broadcast:
                return this->HandlerBroadcast(message);
        }
        return XCode::Successful;
    }

	int DispatchMessageComponent::HandlerForward(const std::shared_ptr<Rpc::Packet>& message)
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

	int DispatchMessageComponent::HandlerResponse(const std::shared_ptr<Rpc::Packet>& message)
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

	int DispatchMessageComponent::HandlerBroadcast(const std::shared_ptr<Rpc::Packet>& message)
	{
        if(this->mOuterComponent == nullptr)
        {
            return XCode::Failure;
        }
        message->SetType(Msg::Type::Request);
        this->mOuterComponent->Broadcast(message);
		return XCode::Successful;
	}
}// namespace Sentry
