#include"LuaPhysicalService.h"
#include"Lua/Function.h"
#include"Module/LuaModule.h"
#include"Lua/LuaServiceMethod.h"
#include"Component/LuaScriptComponent.h"
#include"Config/ServiceConfig.h"
#include"Method/MethodRegister.h"
namespace Sentry
{
	LuaPhysicalService::LuaPhysicalService()
	{
        this->mWaitCount = 0;
        this->mLuaComponent = nullptr;
        this->mIsHandlerMessage = false;
	}

	bool LuaPhysicalService::Start()
	{
        std::vector<const RpcMethodConfig *> rpcInterConfigs;
        this->mMethodRegister = std::make_unique<ServiceMethodRegister>(this);
        const RpcServiceConfig * rpcServiceConfig = RpcConfig::Inst()->GetConfig(this->GetName());
        LOG_CHECK_RET_FALSE(rpcServiceConfig && rpcServiceConfig->GetMethodConfigs(rpcInterConfigs) > 0);
		Lua::LuaModule * luaModule = this->mLuaComponent->GetModule(this->GetName());
		if(luaModule == nullptr)
		{
			return false;
		}

		for(const RpcMethodConfig * rpcInterfaceConfig : rpcInterConfigs)
		{
			if(!luaModule->GetFunction(rpcInterfaceConfig->Method))
			{
                LOG_ERROR("not find lua rpc method [" << rpcInterfaceConfig->FullName << "]");
				return false;
			}
			this->mMethodRegister->AddMethod(
                    std::make_shared<LuaServiceMethod>(rpcInterfaceConfig));
		}
        if (!luaModule->Start())
        {
            return false;
        }        
        this->mIsHandlerMessage = true;
        return true;
	}
	XCode LuaPhysicalService::Invoke(const std::string &name, std::shared_ptr<Rpc::Packet> message)
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

    void LuaPhysicalService::WaitAllMessageComplete()
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

	bool LuaPhysicalService::LateAwake()
	{
		LOG_CHECK_RET_FALSE(RpcService::LateAwake());
		this->mLuaComponent = this->GetComponent<LuaScriptComponent>();
        Lua::LuaModule* luaModule = this->mLuaComponent->LoadModule(this->GetName());
        if(luaModule == nullptr)
        {
            LOG_ERROR("load lua rpc module error : " << this->GetName());
            return false;
        }
		return luaModule->Awake();
	}

	bool LuaPhysicalService::LoadFromLua()
	{
		return true;
	}

	bool LuaPhysicalService::Close()
	{
        Lua::LuaModule* luaModule = this->mLuaComponent->GetModule(this->GetName());
        return luaModule == nullptr || luaModule->Close();
	}
}