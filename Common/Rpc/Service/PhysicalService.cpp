//
// Created by mac on 2022/4/6.
//

#include "Method/MethodRegister.h"

#include"PhysicalService.h"
#include"App/App.h"
#include"Module/LuaModule.h"
#include"Config/ServiceConfig.h"
#include"Lua/LuaServiceMethod.h"
#include"Component/LuaScriptComponent.h"
namespace Sentry
{
    extern std::string GET_FUNC_NAME(const std::string& fullName)
    {
        size_t pos = fullName.find("::");
        return fullName.substr(pos + 2);
    }

    PhysicalService::PhysicalService()
    {
        this->mSumCount = 0;
        this->mWaitCount = 0;
        this->mIsHandlerMessage = false;
    }

    void PhysicalService::WaitAllMessageComplete()
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
    int PhysicalService::Invoke(const std::string &func, std::shared_ptr<Rpc::Packet> message)
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
        this->mSumCount++;
        this->mWaitCount++;
        int code = serviceMethod->Invoke(*message);
        {
            this->mWaitCount--;
        }
        return code;
    }

	bool PhysicalService::Start()
	{
        this->mMethodRegister = std::make_unique<ServiceMethodRegister>(this);
        const RpcServiceConfig * rpcServiceConfig = RpcConfig::Inst()->GetConfig(this->GetName());
		if(rpcServiceConfig == nullptr)
        {
            LOG_ERROR("not find rpc service config " << this->GetName())
            return false;
        }
        if(!this->OnStart())
        {
            LOG_ERROR("rpc service [" << this->GetName() << "] start error");
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

    void PhysicalService::OnRecord(Json::Writer&document)
    {    
        document.Add("sum").Add(this->mSumCount);
        document.Add("wait").Add(this->mWaitCount);
    }

    bool PhysicalService::LoadFromLua()
	{
		LuaScriptComponent* luaScriptComponent = this->GetComponent<LuaScriptComponent>();       
        if (luaScriptComponent == nullptr)
        {
            return false;
        }
		Lua::LuaModule* luaModule = luaScriptComponent->GetModule(this->GetName());
		if (luaModule == nullptr)
		{
            luaModule = luaScriptComponent->LoadModule(this->GetName());
            if(luaModule == nullptr || !luaModule->Awake())
            {
                return false;
            }
		}
        std::vector<const RpcMethodConfig*> methodConfigs;
		const RpcServiceConfig* rpcServiceConfig = RpcConfig::Inst()->GetConfig(this->GetName());
        if (rpcServiceConfig == nullptr)
        {
            return false;
        }
		rpcServiceConfig->GetMethodConfigs(methodConfigs);
		for (const RpcMethodConfig* methodConfig : methodConfigs)
		{
            std::shared_ptr<ServiceMethod> serviceMethod =  
                this->mMethodRegister->GetMethod(methodConfig->Method);
            if (serviceMethod != nullptr && serviceMethod->IsLuaMethod())
            {
                continue;
            }
			if(luaModule->GetFunction(methodConfig->Method))
			{
				std::shared_ptr<LuaServiceMethod> luaServiceMethod
					= std::make_shared<LuaServiceMethod>(methodConfig);
				this->mMethodRegister->AddMethod(luaServiceMethod);
				LOG_WARN(methodConfig->FullName << " use lua method");
			}
		}
	}

	bool PhysicalService::Close()
    {
        if (!this->IsStartService())
        {
            return false;
        }
        this->OnClose();
        return true;
    }
}