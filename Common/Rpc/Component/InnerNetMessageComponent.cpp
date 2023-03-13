#include"InnerNetMessageComponent.h"
#include"Component/TaskComponent.h"
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
	bool InnerNetMessageComponent::Awake()
	{
		this->mTaskComponent = nullptr;
		this->mInnerComponent = nullptr;
        return true;
	}

	bool InnerNetMessageComponent::LateAwake()
	{
		this->mTaskComponent = this->mApp->GetTaskComponent();
        this->mTimerComponent = this->mApp->GetTimerComponent();
        this->mOuterComponent = this->GetComponent<OuterNetComponent>();
		LOG_CHECK_RET_FALSE(this->mTaskComponent = this->GetComponent<TaskComponent>());
		LOG_CHECK_RET_FALSE(this->mInnerComponent = this->GetComponent<InnerNetComponent>());
		return true;
	}

	int InnerNetMessageComponent::OnRequest(std::shared_ptr<Rpc::Packet> message)
	{
        const Rpc::Head & head = message->GetHead();
        if(!head.Get("func", this->mFullName))
        {
            return XCode::CallArgsError;
        }
        const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(this->mFullName);
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

    void InnerNetMessageComponent::Send(const std::string& address, int code, std::shared_ptr<Rpc::Packet> message)
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

    void InnerNetMessageComponent::Invoke(const RpcMethodConfig *config, std::shared_ptr<Rpc::Packet> message)
    {
        RpcService *logicService = this->mApp->GetService(config->Service);
        if (logicService == nullptr || !logicService->IsStartService())
        {
            LOG_ERROR("call [" << config->FullName << "] server not found");
            return;
        }
        long long timerId = 0;
        if (config->Timeout > 0)
        {
            timerId = this->mTimerComponent->DelayCall(config->Timeout, []()
                {

                }
            );
        }
        int code = logicService->Invoke(config->Method, message);
        if (timerId > 0 && !this->mTimerComponent->CancelTimer(timerId))
        {
            LOG_ERROR("call [" << config->FullName << "] time out not return");
            return;
        }
        if (code != XCode::Successful)
        {
            LOG_ERROR("call [" << config->FullName << "] time out not return");
        }
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
        const std::string &address, std::shared_ptr<Rpc::Packet> message)
    {
        int rpcId = this->mNuberPool.Pop();
        message->GetHead().Remove("address");
        std::shared_ptr<RpcTaskSource> taskSource = std::make_shared<RpcTaskSource>(rpcId);
        {
            message->GetHead().Add("rpc", rpcId);
        }
        if (!this->mInnerComponent->Send(address, message))
        {
            return nullptr;
        }
        std::shared_ptr<Rpc::Packet> response = this->AddTask(rpcId, taskSource)->Await();
        {
            this->mNuberPool.Push(rpcId);
        }
        return response;
    }

    void InnerNetMessageComponent::OnMessage(std::shared_ptr<Rpc::Packet> message)
    {
        Tcp::Type type = (Tcp::Type)message->GetType();
        switch (type)
        {
        case Tcp::Type::Request:
        {
            int code = this->OnRequest(message);
            if (code != XCode::Successful)
            {
                message->Clear();
                const std::string& address = message->From();
                this->Send(address, code, std::move(message));
            }
            break;
        } 
        case Tcp::Type::Response:
        {
            if (this->mOuterComponent != nullptr)
            {
                std::string target;
                // 网关转发过来的消息 必须带client字段
                if (message->GetHead().Get("client", target))
                {
                    message->SetFrom(target);
                    message->GetHead().Remove("client");
                    this->mOuterComponent->OnMessage(message);
                    return;
                }
            }
            long long rpcId = 0;
            if (message->GetHead().Get("rpc", rpcId))
            {
                this->OnResponse(rpcId, message);
                return;
            }
            break;
        }
        }
    }

    bool InnerNetMessageComponent::Send(const std::string &address, std::shared_ptr<Rpc::Packet> message)
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
}// namespace Sentry
