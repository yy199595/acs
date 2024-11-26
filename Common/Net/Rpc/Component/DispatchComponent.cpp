#include"DispatchComponent.h"

#include"Async/Component/CoroutineComponent.h"
#include"Rpc/Config/ServiceConfig.h"
#include"Timer/Timer/ElapsedTimer.h"
#ifdef __DEBUG__
#include"Server/Config/CodeConfig.h"
#include"Proto/Component/ProtoComponent.h"
#endif
#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Rpc/Client/Message.h"
#include"Rpc/Service/RpcService.h"
#include"Gate/Component/OuterNetComponent.h"
#include"Router/Component/RouterComponent.h"

#include"Core/System/System.h"

namespace acs
{
	DispatchComponent::DispatchComponent()
	{
		this->mSumCount = 0;
		this->mWaitCount = 0;
		this->mTaskComponent = nullptr;
		this->mOuterComponent = nullptr;
		this->mRouterComponent = nullptr;
	}

	bool DispatchComponent::LateAwake()
	{
		std::vector<RpcService *> rpcServices;
		this->mApp->GetComponents(rpcServices);
		for(RpcService * rpcService : rpcServices)
		{
			const std::string & name = rpcService->GetName();
			LOG_CHECK_RET_FALSE(this->mRpcServices.Add(name, rpcService));
		}

		this->mTaskComponent = App::Coroutine();
		this->mRouterComponent = this->GetComponent<RouterComponent>();
		this->mOuterComponent = this->GetComponent<OuterNetComponent>();
		return true;
	}

	void DispatchComponent::OnAppStop()
	{
		while(this->mWaitCount > 1)
		{
			this->mApp->Sleep();
			LOG_INFO("wait message count:{}", this->mWaitCount);
		}
	}

	int DispatchComponent::OnRequest(rpc::Packet * message)
	{
		++this->mSumCount;
		int code = XCode::Ok;
		const std::string & fullName = message->ConstHead().GetStr("func");
        const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);

		do
		{
			if(methodConfig == nullptr)
			{
				code = XCode::CallFunctionNotExist;
				break;
			}
//#ifdef __DEBUG__
//			std::string fromAddress = "unknown";
//			message->TempHead().Del(rpc::Header::from_addr, fromAddress);
//			LOG_DEBUG("[{}://{}] call ({})", methodConfig->NetName, fromAddress, methodConfig->FullName);
//#endif
			//LOG_DEBUG("call ({}) by {}", fullName, methodConfig->NetName);
			if(!this->mRpcServices.Has(methodConfig->Service))
			{
				code = XCode::CallServiceNotFound;
				LOG_ERROR("call {} not exist", methodConfig->Service);
				break;
			}
			if (!methodConfig->IsAsync)
			{
				this->Invoke(methodConfig, message);
				break;
			}
			this->mTaskComponent->Start(&DispatchComponent::Invoke, this, methodConfig, message);
		}
		while(false);
		return code;
	}

    void DispatchComponent::Invoke(const RpcMethodConfig *config, rpc::Packet * message)
	{
		++this->mWaitCount;
		int code = XCode::Ok;
#ifdef __DEBUG__
		long long start = help::Time::NowMil();
#endif
		do
		{
			const std::string& service = config->Service;
			RpcService* logicService = this->mRpcServices.Find(service);
			if (logicService == nullptr)
			{
				code = XCode::CallServiceNotFound;
				LOG_ERROR("call {} server not found", config->FullName);
				break;
			}
			code = logicService->Invoke(config, message);
		} while (false);
#ifdef __DEBUG__
		long long t = help::Time::NowMil() - start;
		if (code != XCode::Ok)
		{
			const std::string& desc = CodeConfig::Inst()->GetDesc(code);
			//LOG_WARN("({}ms) invoke [{}] code:{} = {}", t, config->FullName, code, desc);
		}
		else if(t >= 2000)
		{
			LOG_WARN("({}ms) invoke [{}] too long time", t, config->FullName);
		}
#endif
		--this->mWaitCount;
		this->mRouterComponent->Send(message->SockId(), code, message);
	}

	void DispatchComponent::OnRecord(json::w::Document& document)
	{
		std::unique_ptr<json::w::Value> data = document.AddObject("dispatch");
		{
			data->Add("sum", this->mSumCount);
			data->Add("wait", this->mWaitCount);
			data->Add("rpc_wait", this->AwaitCount());
		}
	}

    int DispatchComponent::OnMessage(rpc::Packet * message)
    {
        switch (message->GetType())
        {
            case rpc::Type::Request:
                return this->OnRequest(message);
            case rpc::Type::Response:
			{
				int rpcId = message->GetRpcId();
				this->OnResponse(rpcId, message);
				return XCode::Ok;
			}
			case rpc::Type::Client:
				return this->OnClient(message);
            case rpc::Type::Broadcast:
                return this->OnBroadcast(message);
        }
		LOG_ERROR("unknown message type : {}", message->GetType());
        return XCode::UnKnowPacket;
    }

	int DispatchComponent::OnClient(rpc::Packet * message)
	{
		if (this->mOuterComponent == nullptr)
		{
			return XCode::NetWorkError;
		}
		long long userId = 0;
		if (!message->GetHead().Get(rpc::Header::player_id, userId))
		{
			LOG_ERROR("not find userid forward to client fail");
			return XCode::CallArgsError;
		}
		message->SetType(rpc::Type::Request);
		message->GetHead().Del(rpc::Header::player_id);
		this->mOuterComponent->SendToPlayer(userId, message);
		return XCode::Ok;
	}

	int DispatchComponent::OnBroadcast(rpc::Packet * message)
	{
        if(this->mOuterComponent == nullptr)
        {
            return XCode::Failure;
        }
        message->SetType(rpc::Type::Request);
        this->mOuterComponent->Broadcast(message);
		return XCode::Ok;
	}
}// namespace Sentry
