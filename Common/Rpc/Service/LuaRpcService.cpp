#include"LuaRpcService.h"
#include"Lua/Table.h"
#include"Lua/Function.h"
#include"Lua/LuaServiceMethod.h"
#include"Component/LuaScriptComponent.h"
#include"Config/ServiceConfig.h"
#include"Method/MethodRegister.h"
namespace Sentry
{
	LuaRpcService::LuaRpcService()
	{
        this->mWaitCount = 0;
        this->mIsHandlerMessage = false;
	}

	bool LuaRpcService::Start()
	{
        std::string location;
        const ServerConfig * config = ServerConfig::Inst();
        LOG_CHECK_RET_FALSE(config->GetLocation("rpc", location));
		this->mEventRegister = std::make_shared<NetEventRegistry>(this);
		this->mMethodRegister = std::make_shared<ServiceMethodRegister>(this);
        const RpcServiceConfig * rpcServiceConfig = RpcConfig::Inst()->GetConfig(this->GetName());

		std::vector<const RpcMethodConfig *> rpcInterConfigs;
        LOG_CHECK_RET_FALSE(rpcServiceConfig->GetMethodConfigs(rpcInterConfigs) > 0);

		for(const RpcMethodConfig * rpcInterfaceConfig : rpcInterConfigs)
		{
            const std::string & tab = rpcInterfaceConfig->Service;
            const std::string & func = rpcInterfaceConfig->Method;
            if(!this->mLuaComponent->GetFunction(tab, func))
            {
                LOG_ERROR("not find rpc method = [" << tab << '.' << func << ']');
            }
			if (!this->mMethodRegister->AddMethod(std::make_shared<LuaServiceMethod>
			        (rpcInterfaceConfig, this->mLuaComponent->GetLuaEnv())))
			{
				return false;
			}
		}
        std::vector<IServiceChange *> components;
        this->mApp->GetComponents<IServiceChange>(components);
        {
            for (IServiceChange *component: components)
            {
                if(!component->OnStartService(this->GetName()))
                {
                    return false;
                }
            }
        }
        this->mIsHandlerMessage = true;
        return true;
	}
#ifdef __ENABLE_REDIS__
	XCode LuaRpcService::Invoke(const std::string& id, const std::string& message)
	{
		return XCode::Successful;
	}
#endif

	XCode LuaRpcService::Invoke(const std::string &name, std::shared_ptr<Rpc::Packet> message)
	{
		if(!this->IsStartService())
		{
			LOG_ERROR(this->GetName() << " is not start");
			return XCode::CallServiceNotFound;
		}
        if(!this->mIsHandlerMessage)
        {
            return XCode::CallServiceNotFound;
        }
		std::shared_ptr<ServiceMethod> serviceMethod = this->mMethodRegister->GetMethod(name);
		if (serviceMethod == nullptr)
		{
			return XCode::CallServiceNotFound;
		}
        this->mWaitCount++;
		XCode code = serviceMethod->Invoke(*message);
        this->mWaitCount--;
        return code;
	}

    void LuaRpcService::WaitAllMessageComplete()
    {
        this->mIsHandlerMessage = false;
        TaskComponent *taskComponent = this->mApp->GetTaskComponent();
        while (this->mWaitCount > 0)
        {
            taskComponent->Sleep(100);
            CONSOLE_LOG_INFO(this->GetName() <<
                " wait handler message count [" << this->mWaitCount << "]");
        }
        CONSOLE_LOG_ERROR(this->GetName() << " handler all message complete");
    }

	bool LuaRpcService::LateAwake()
	{
		LOG_CHECK_RET_FALSE(RpcService::LateAwake());
		this->mLuaComponent = this->GetComponent<LuaScriptComponent>();
        if(!this->mLuaComponent->LoadModule(this->GetName()))
        {
            LOG_ERROR("load lua rpc module error : " << this->GetName());
            return false;
        }
		return true;
	}

	bool LuaRpcService::Close()
	{
        std::vector<IServiceChange *> components;
        this->mApp->GetComponents<IServiceChange>(components);
        {
            for (IServiceChange *component: components)
            {
                if(!component->OnCloseService(this->GetName()))
                {
                    return false;
                }
            }
        }
        return true;
	}
#ifdef __ENABLE_REDIS__
	void LuaRpcService::GetSubEventIds(std::unordered_set<std::string>& evendIds)
	{
		if(this->mMethodRegister != nullptr)
		{
			std::vector<std::string> rets;
			this->mEventRegister->GetEvents(rets);
			for(const std::string & eventId : rets)
			{
				evendIds.insert(eventId);
			}
		}
	}
#endif
}