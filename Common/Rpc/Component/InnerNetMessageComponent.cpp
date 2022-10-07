#include"InnerNetMessageComponent.h"
#include"Component/TaskComponent.h"
#include"App/App.h"
#include"Lua/LuaServiceMethod.h"
#include"Config/ServiceConfig.h"
#include"InnerNetComponent.h"
#include"Timer/ElapsedTimer.h"
#include"Service/LocalService.h"
#include"Async/RpcTaskSource.h"
namespace Sentry
{
	void InnerNetMessageComponent::Awake()
	{
		this->mTaskComponent = nullptr;
		this->mTimerComponent = nullptr;
		this->mRpcClientComponent = nullptr;
	}

	bool InnerNetMessageComponent::LateAwake()
	{
		this->mTaskComponent = App::Get()->GetTaskComponent();
		this->mTimerComponent = this->GetComponent<TimerComponent>();
		LOG_CHECK_RET_FALSE(this->mTaskComponent = this->GetComponent<TaskComponent>());
		LOG_CHECK_RET_FALSE(this->mRpcClientComponent = this->GetComponent<InnerNetComponent>());
		return true;
	}

	XCode InnerNetMessageComponent::OnRequest(std::shared_ptr<Rpc::Data> message)
	{
        std::string func, address;
        const Rpc::Head & head = message->GetHead();
        LOG_RPC_CHECK_ARGS(head.Get("func", func));
        LOG_RPC_CHECK_ARGS(head.Get("address", address));
        LOG_RPC_CHECK_ARGS(message->GetMethod(this->mService, this->mMethod));

		Service * logicService = this->GetApp()->GetService(this->mService);
		if (logicService == nullptr || !logicService->IsStartService())
		{
            LOG_ERROR("call service not exist : [" << this->mService << "]");
			return XCode::CallServiceNotFound;
		}
		const RpcServiceConfig & rpcServiceConfig = logicService->GetServiceConfig();
		const RpcMethodConfig* methodConfig = rpcServiceConfig.GetConfig(this->mMethod);
		if (methodConfig == nullptr)
		{
            return XCode::NotFoundRpcConfig;
		}
		if(!methodConfig->Request.empty() && message->GetSize() == 0)
        {
            return XCode::CallArgsError;
        }

		if (!methodConfig->IsAsync)
        {
            this->Invoke(methodConfig, message);
            return XCode::Successful;
        }
        this->mTaskComponent->Start(&InnerNetMessageComponent::Invoke, this, methodConfig, message);
		return XCode::Successful;
	}

    void InnerNetMessageComponent::Invoke(const RpcMethodConfig *config, std::shared_ptr<Rpc::Data> message)
    {
        XCode code = XCode::Failure;
        Service * logicService = this->GetApp()->GetService(config->Service);
        try
        {
            code = logicService->Invoke(config->Method, message);
        }
        catch (std::exception & e)
        {
            code = XCode::ThrowError;
            message->GetHead().Add("error", e.what());
        }
        std::string address;
        message->GetHead().Add("code", code);
        if(message->GetHead().Get("address", address))
        {
            message->GetHead().Remove("address");
            if(message->GetHead().Has("rpc"))
            {
                message->SetType(Tcp::Type::Response);
                this->mRpcClientComponent->Send(address, message);
            }
        }
    }

    std::shared_ptr<Rpc::Data> InnerNetMessageComponent::Call(
        const std::string &address, std::shared_ptr<Rpc::Data> message)
    {
        message->GetHead().Remove("address");
        if (!this->mRpcClientComponent->Send(address, message))
        {
            return nullptr;
        }
        std::shared_ptr<RpcTaskSource> taskSource =
            std::make_shared<RpcTaskSource>(0);
        return this->AddTask(taskSource)->Await();
    }

    bool InnerNetMessageComponent::Send(const std::string &address, std::shared_ptr<Rpc::Data> message)
    {
        message->GetHead().Remove("address");
        assert(message->GetType() != (int)Tcp::Type::None);
        assert(message->GetProto() != (int)Tcp::Porto::None);
        return this->mRpcClientComponent->Send(address, message);
    }
}// namespace Sentry
