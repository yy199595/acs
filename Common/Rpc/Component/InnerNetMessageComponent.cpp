#include"InnerNetMessageComponent.h"
#include"Component/TaskComponent.h"
#include"Lua/LuaServiceMethod.h"
#include"Config/ServiceConfig.h"
#include"InnerNetComponent.h"
#include"Timer/ElapsedTimer.h"
#include"Service/LocalRpcService.h"
#include"Async/RpcTaskSource.h"
#include"Config/CodeConfig.h"
namespace Sentry
{
	bool InnerNetMessageComponent::Awake()
	{
		this->mTaskComponent = nullptr;
		this->mRpcClientComponent = nullptr;
        return true;
	}

	bool InnerNetMessageComponent::LateAwake()
	{
		this->mTaskComponent = this->mApp->GetTaskComponent();
		LOG_CHECK_RET_FALSE(this->mTaskComponent = this->GetComponent<TaskComponent>());
		LOG_CHECK_RET_FALSE(this->mRpcClientComponent = this->GetComponent<InnerNetComponent>());
		return true;
	}

	XCode InnerNetMessageComponent::OnRequest(std::shared_ptr<Rpc::Packet> message)
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
        if(!methodConfig->Request.empty() && message->GetSize() == 0)
        {
            return XCode::CallArgsError;
        }
        if(this->mApp->GetService(methodConfig->Service) == nullptr)
        {
            LOG_ERROR("call service not exist : [" << methodConfig->Service << "]");
            return XCode::CallServiceNotFound;
        }
        this->mWaitMessages.push(std::move(message));
		return XCode::Successful;
	}

    void InnerNetMessageComponent::OnFrameUpdate(float t)
    {
        size_t count = 0;
        while(!this->mWaitMessages.empty() && count <= MAX_HANDLER_MSG_COUNT)
        {
            count++;
            std::shared_ptr<Rpc::Packet> message = this->mWaitMessages.front();
			this->mWaitMessages.pop();
            if(message->GetHead().Get("func", this->mFullName))
            {
                const RpcMethodConfig *methodConfig = RpcConfig::Inst()->GetMethodConfig(this->mFullName);
                if (methodConfig != nullptr)
                {
                    if (!methodConfig->IsAsync)
                    {
                        this->Invoke(methodConfig, message);
                        continue;
                    }
                    this->mTaskComponent->Start(&InnerNetMessageComponent::Invoke,
                                                this, methodConfig, message);
                }
            }
        }
    }

    void InnerNetMessageComponent::Invoke(const RpcMethodConfig *config, std::shared_ptr<Rpc::Packet> message)
    {
        Rpc::Head & head = message->GetHead();
        RpcService *logicService = this->mApp->GetService(config->Service);
        LOG_CHECK_RET(logicService != nullptr);
        XCode code = logicService->Invoke(config->Method, message);
        if (code != XCode::Successful)
        {
            head.Add("error", CodeConfig::Inst()->GetDesc(code));
        }
#ifdef __DEBUG__
        std::string from;
        if (head.Get("address", from))
        {
            const ServiceNodeInfo *nodeInfo = this->mRpcClientComponent->GetSeverInfo(from);
            if (nodeInfo != nullptr)
            {
                from = nodeInfo->LocationRpc;
            }
            if (code != XCode::Successful)
            {
                CONSOLE_LOG_ERROR("[" << from << "] call ["
                                      << config->FullName << "] code = " << CodeConfig::Inst()->GetDesc(code));
            }
        }
#endif
        if (!head.Has("rpc"))
        {
            return; //不需要返回
        }
        std::string address;
        head.Add("code", code);
        if (head.Get("address", address))
        {
            head.Remove("id");
            head.Remove("address");
#ifndef __DEBUG__
            head.Remove("func");
#endif
            message->SetType(Tcp::Type::Response);
            this->mRpcClientComponent->Send(address, message);
        }
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
        message->GetHead().Remove("address");
        assert(message->GetType() < (int)Tcp::Type::Max);
        assert(message->GetType() > (int)Tcp::Type::None);
        std::shared_ptr<RpcTaskSource> taskSource = std::make_shared<RpcTaskSource>(0);
        {
            message->GetHead().Add("rpc", taskSource->GetRpcId());
        }
        if (!this->mRpcClientComponent->Send(address, message))
        {
            return nullptr;
        }
        return this->AddTask(taskSource->GetRpcId(), taskSource)->Await();
    }

    bool InnerNetMessageComponent::Send(const std::string &address, std::shared_ptr<Rpc::Packet> message)
    {
        if (this->mRpcClientComponent == nullptr)
        {
            return false;
        }
        message->GetHead().Remove("address");
        assert(message->GetType() < (int)Tcp::Type::Max);
        assert(message->GetType() > (int)Tcp::Type::None);
        return this->mRpcClientComponent->Send(address, message);
    }
}// namespace Sentry
