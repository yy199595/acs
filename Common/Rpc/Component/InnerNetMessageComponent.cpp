#include"InnerNetMessageComponent.h"

#include <utility>
#include"Component/AsyncMgrComponent.h"
#include"Lua/LuaServiceMethod.h"
#include"Config/ServiceConfig.h"
#include"InnerNetComponent.h"
#include"Timer/ElapsedTimer.h"
#include"Service/PhysicalService.h"
#include"Async/RpcTaskSource.h"
#include"Config/CodeConfig.h"
#include"Component/OuterNetComponent.h"
namespace Sentry
{
	InnerNetMessageComponent::InnerNetMessageComponent()
	{
		this->mWaitCount = 0;
		this->mTaskComponent = nullptr;
		this->mInnerComponent = nullptr;
		this->mOuterComponent = nullptr;
		this->mTimerComponent = nullptr;
	}

	bool InnerNetMessageComponent::LateAwake()
	{
        this->mTimerComponent = this->mApp->GetTimerComponent();
        this->mOuterComponent = this->GetComponent<OuterNetComponent>();
		LOG_CHECK_RET_FALSE(this->mTaskComponent = this->GetComponent<AsyncMgrComponent>());
		LOG_CHECK_RET_FALSE(this->mInnerComponent = this->GetComponent<InnerNetComponent>());
		return true;
	}

	int InnerNetMessageComponent::HandlerRequest(std::shared_ptr<Rpc::Packet> message)
	{
        const Rpc::Head & head = message->ConstHead();
		const std::string & fullName = message->GetHead().GetStr("func");
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
        this->mTaskComponent->Start(&InnerNetMessageComponent::Invoke, this, methodConfig, message);
		return XCode::Successful;
	}

    void InnerNetMessageComponent::Send(const std::string& address, int code, const std::shared_ptr<Rpc::Packet>& message)
    { 
        if (!message->GetHead().Has("rpc"))
        {
            return; //不需要返回
        }
        message->GetHead().Remove("id");
        message->GetHead().Add("code", code);
#ifndef __DEBUG__
        head.Remove("func");
#endif
        message->SetType(Tcp::Type::Response);
        this->mInnerComponent->Send(message->From(), message);
    }

    void InnerNetMessageComponent::Invoke(const RpcMethodConfig *config, const std::shared_ptr<Rpc::Packet>& message)
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
				  message->GetHead().Add("code", XCode::CallTimeout);
				  LOG_ERROR("call [" << config->FullName << "] code = call time out");
				  this->Send(message->From(), XCode::CallTimeout, message);
				}
			);
		}
#ifdef __DEBUG__
	ElapsedTimer timer;
#endif
		this->mWaitCount++;
		int code = logicService->Invoke(config->Method, message);
#ifdef __DEBUG__
		LOG_INFO("call [" << config->FullName << "] use time = " << timer.GetMs() << "ms");
#endif
		this->mWaitCount--;
		if (timerId > 0 && !this->mTimerComponent->CancelTimer(timerId))
		{
			LOG_ERROR("call [" << config->FullName << "] time out not return");
			return;
		}
		if(code != XCode::Successful)
		{
			const std::string& desc = CodeConfig::Inst()->GetDesc(code);
			CONSOLE_LOG_INFO("call [" << config->FullName << "] code = " << desc);
		}
		message->SetType(Tcp::Type::Response);
		this->Send(message->From(), code, message);
	}


    bool InnerNetMessageComponent::Ping(const std::string &address)
    {
        std::shared_ptr<Rpc::Packet> message = std::make_shared<Rpc::Packet>();
        {
            message->SetType(Tcp::Type::Ping);
        }
        std::shared_ptr<Rpc::Packet> response = this->Call(address, message);
        return response->GetCode(XCode::Failure) == XCode::Successful;
    }

    std::shared_ptr<Rpc::Packet> InnerNetMessageComponent::Call(
        const std::string &address, const std::shared_ptr<Rpc::Packet>& message)
    {
        int rpcId = 0;
		if(!this->Send(address, message, rpcId))
		{
			return nullptr;
		}
        std::shared_ptr<RpcTaskSource> taskSource =
			std::make_shared<RpcTaskSource>(rpcId);
        return this->AddTask(rpcId, taskSource)->Await();
    }

    int InnerNetMessageComponent::OnMessage(const std::shared_ptr<Rpc::Packet>& message)
    {
        switch (message->GetType())
        {
        case Tcp::Type::Request:
            return this->HandlerRequest(message);
        case Tcp::Type::Response:
			return this->HandlerResponse(message);
		case Tcp::Type::Forward:
			return this->HandlerForward(message);
		case Tcp::Type::Broadcast:
			return this->HandlerBroadcast(message);
        }
		return XCode::Successful;
    }

	int InnerNetMessageComponent::HandlerForward(const std::shared_ptr<Rpc::Packet>& message)
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
		message->SetType(Tcp::Type::Request);
		if(!this->mOuterComponent->Send(userId, message))
		{
			return XCode::NotFindUser;
		}
		return XCode::Successful;
	}

	int InnerNetMessageComponent::HandlerResponse(const std::shared_ptr<Rpc::Packet>& message)
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
		if (message->GetHead().Get("rpc", rpcId))
		{
			this->OnResponse(rpcId, message);
			return XCode::Successful;
		}
		return XCode::Successful;
	}

	int InnerNetMessageComponent::HandlerBroadcast(const std::shared_ptr<Rpc::Packet>& message)
	{
        if(this->mOuterComponent == nullptr)
        {
            return XCode::Failure;
        }
        message->SetType(Tcp::Type::Request);
        this->mOuterComponent->Broadcast(message);
		return XCode::Successful;
	}


	bool InnerNetMessageComponent::Send(const std::string &address, const std::shared_ptr<Rpc::Packet>& message)
    {
        if (this->mInnerComponent == nullptr)
        {
            return false;
        }
        message->GetHead().Remove("address");
        assert(message->GetType() < (int)Tcp::Type::Max);
        assert(message->GetType() > (int)Tcp::Type::None);
        return this->mInnerComponent->Send(address, message);
    }
	bool InnerNetMessageComponent::Send(const string& address, const std::shared_ptr<Rpc::Packet>& message, int& id)
	{
		int rpcId = this->mNumberPool.Pop();
		message->GetHead().Add("rpc", rpcId);
		if(!this->Send(address, message))
		{
			this->mNumberPool.Push(rpcId);
			return false;
		}
		id = rpcId;
		return true;
	}
}// namespace Sentry
