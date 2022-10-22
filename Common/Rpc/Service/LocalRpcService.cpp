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
    void ServiceRunInfo::OnCall(const std::string &func)
    {

    }

    void ServiceRunInfo::OnInvokeCompete(const std::string &func, int ms)
    {

    }
}

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

	XCode LocalRpcService::Invoke(const string& id, const string& message)
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
		std::shared_ptr<EventMethod> method = this->mEventRegister->GetEvent(id);
		if(method != nullptr)
		{
			method->Run(message);
			return XCode::Successful;
		}
		return XCode::CallFunctionNotExist;
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
		this->mEventRegister = std::make_shared<NetEventRegistry>(this);
		this->mMethodRegister = std::make_shared<ServiceMethodRegister>(this);
        const RpcServiceConfig * rpcServiceConfig = RpcConfig::Inst()->GetConfig(this->GetName());
		if (!this->OnStart())
		{
			return false;
		}
        this->LoadFromLua();
        std::vector<const RpcMethodConfig *> methodConfigs;
        rpcServiceConfig->GetMethodConfigs(methodConfigs);
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

    void LocalRpcService::OnHotFix()
    {
        this->mMethodRegister->ClearLuaMethods();
        this->LoadFromLua();
    }

    void LocalRpcService::LoadFromLua()
    {
        std::vector<const RpcMethodConfig *> methodConfigs;
        LuaScriptComponent* luaScriptComponent = this->GetComponent<LuaScriptComponent>();
        const RpcServiceConfig * rpcServiceConfig = RpcConfig::Inst()->GetConfig(this->GetName());
        if(luaScriptComponent != nullptr && luaScriptComponent->LoadModule(this->GetName()))
        {
            lua_State *lua = luaScriptComponent->GetLuaEnv();
            rpcServiceConfig->GetMethodConfigs(methodConfigs);
            for (const RpcMethodConfig *methodConfig: methodConfigs)
            {
                const std::string & tab = methodConfig->Service;
                const std::string & func = methodConfig->Method;
                if (luaScriptComponent->GetFunction(tab, func))
                {
                    std::shared_ptr<LuaServiceMethod> luaServiceMethod
                        = std::make_shared<LuaServiceMethod>(methodConfig, lua);
                    this->mMethodRegister->AddMethod(luaServiceMethod);
                    LOG_WARN(methodConfig->FullName << " use lua method");
                }
            }
        }
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