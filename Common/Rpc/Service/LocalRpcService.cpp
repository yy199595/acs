//
// Created by mac on 2022/4/6.
//

#include "Method/MethodRegister.h"

#include"LocalRpcService.h"
#include"App/App.h"
#include"Config/ServiceConfig.h"
#include"Lua/LuaServiceMethod.h"
#include"Component/InnerNetComponent.h"
#include"Component/LuaScriptComponent.h"

namespace Sentry
{
    extern std::string GET_FUNC_NAME(std::string fullName)
    {
        size_t pos = fullName.find("::");
        return fullName.substr(pos + 2);
    }

    LocalRpcService::LocalRpcService()
    {
        this->mWaitCount = 0;
        this->mIsHandlerMessage = false;
    }

    void LocalRpcService::WaitAllMessageComplete()
    {
        int time = 0;
        this->mIsHandlerMessage = false;
        TaskComponent *taskComponent = this->mApp->GetTaskComponent();
        while (this->mWaitCount > 0)
        {
            time++;
            taskComponent->Sleep(1000);
            CONSOLE_LOG_DEBUG(this->GetName() <<
                " wait handler message count [" << this->mWaitCount << "] time = [" << time << "s]");
        }
        CONSOLE_LOG_ERROR(this->GetName() << " handler all message complete");
    }

    XCode LocalRpcService::Invoke(const std::string &func, std::shared_ptr<Rpc::Packet> message)
    {
        if (!this->IsStartService())
        {
            LOG_ERROR(this->GetName() << " is not start");
            return XCode::CallServiceNotFound;
        }
        if(!this->mIsHandlerMessage)
        {
            return XCode::CallServiceNotFound;
        }
        std::shared_ptr<ServiceMethod> serviceMethod = this->mMethodRegister->GetMethod(func);
        if (serviceMethod == nullptr)
        {
            LOG_ERROR("not find [" << this->GetName() << "." << func << "]");
            return XCode::CallServiceNotFound;
        }
#ifndef __DEBUG__
        message->GetHead().Remove("func");
#endif
        this->mWaitCount++;
        XCode code = serviceMethod->Invoke(*message);
        this->mWaitCount--;
        return code;
    }

	bool LocalRpcService::Start()
	{
		this->mMethodRegister = std::make_shared<ServiceMethodRegister>(this);
        const RpcServiceConfig * rpcServiceConfig = RpcConfig::Inst()->GetConfig(this->GetName());
		if (!this->OnStart())
		{
			return false;
		}
        std::vector<const RpcMethodConfig *> methodConfigs;
        rpcServiceConfig->GetMethodConfigs(methodConfigs);
		LuaScriptComponent* luaScriptComponent = this->GetComponent<LuaScriptComponent>();

		if (luaScriptComponent != nullptr)
		{
            lua_State * lua = luaScriptComponent->GetLuaEnv();
            for(const RpcMethodConfig * methodConfig : methodConfigs)
            {
                const char* tab = methodConfig->Service.c_str();
                const char * func = methodConfig->Method.c_str();
                if (Lua::Function::Get(lua, tab, func))
                {
                    std::shared_ptr<LuaServiceMethod> luaServiceMethod
                            = std::make_shared<LuaServiceMethod>(methodConfig, lua);
                    this->mMethodRegister->AddMethod(luaServiceMethod);
                }
                else if (this->mMethodRegister->GetMethod(methodConfig->Method) == nullptr)
                {
                    CONSOLE_LOG_ERROR(this->GetName() << " not register method [" << methodConfig->Method << "]");
                    return false;
                }
            }
		}

        for(const RpcMethodConfig * config : methodConfigs)
        {
            if(this->mMethodRegister->GetMethod(config->Method) == nullptr)
            {
                LOG_ERROR("not register " << config->FullName);
                return false;
            }
        }
        this->mIsHandlerMessage = true;
		return true;
	}

	bool LocalRpcService::Close()
    {
        if (!this->IsStartService() || !this->OnClose())
        {
            return false;
        }
        std::move(this->mMethodRegister);
        return true;
    }
}