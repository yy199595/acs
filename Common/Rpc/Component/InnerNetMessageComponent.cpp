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
		const RpcInterfaceConfig* rpcInterfaceConfig = rpcServiceConfig.GetConfig(this->mMethod);
		if (rpcInterfaceConfig == nullptr)
		{
            return XCode::NotFoundRpcConfig;
		}
		if(!rpcInterfaceConfig->Request.empty() && message->GetSize() == 0)
        {
            return XCode::CallArgsError;
        }

		const std::string& service = rpcInterfaceConfig->Service;
		if (!rpcInterfaceConfig->IsAsync)
        {
            const std::string &func = rpcInterfaceConfig->Method;
            XCode code = logicService->Invoke(func, message);
            if (head.Has("rpc"))
            {
                message->GetHead().Add("code", (int) code);
                this->mRpcClientComponent->Send(address, message);
            }
            return XCode::Successful;
        }

		this->mTaskComponent->Start([message, address, this, logicService, rpcInterfaceConfig]()
		{
			const std::string& func = rpcInterfaceConfig->Method;
			XCode code = logicService->Invoke(func, message);
            if(message->GetHead().Has("rpc"))
            {
                message->GetHead().Add("code", (int) code);
                this->mRpcClientComponent->Send(address, message);
            }
        });
		return XCode::Successful;
	}

    std::shared_ptr<Rpc::Data> InnerNetMessageComponent::Call(
        const std::string &address, std::shared_ptr<Rpc::Data> message)
    {
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
        assert(message->GetType() != Tcp::Type::None);
        assert(message->GetProto() != Tcp::Porto::None);
        std::shared_ptr<InnerNetClient> netClient =
            this->mRpcClientComponent->GetOrCreateSession(address);
        if(netClient == nullptr)
        {
            return false;
        }
        netClient->SendData(message);
        return true;
    }
}// namespace Sentry
