//
// Created by mac on 2022/4/6.
//
#include"PhysicalRpcService.h"
#include "Rpc/Method/MethodRegister.h"


#include"Entity/App/App.h"
#include"Script/Module/LuaModule.h"
#include"Server/Config/ServiceConfig.h"
#include"Rpc/Lua/LuaServiceMethod.h"
#include"Script/Component/LuaScriptComponent.h"
namespace Sentry
{
    extern std::string GET_FUNC_NAME(const std::string& fullName)
    {
        size_t pos = fullName.find("::");
        return fullName.substr(pos + 2);
    }

    PhysicalRpcService::PhysicalRpcService()
	 : mMethodRegister(this)
    {
        this->mSumCount = 0;
        this->mWaitCount = 0;
        this->mIsHandle = false;
    }

    void PhysicalRpcService::WaitAllMessageComplete()
    {
        int time = 0;
        this->mIsHandle = false;
        AsyncMgrComponent *taskComponent = this->mApp->GetTaskComponent();
        while (this->mWaitCount > 0)
        {
            time++;
            taskComponent->Sleep(1000);
            CONSOLE_LOG_DEBUG(this->GetName() <<
                " wait handler message count [" << this->mWaitCount << "] time = [" << time << "s]");
        }
        CONSOLE_LOG_INFO(this->GetName() << " handler all message complete");
    }
    int PhysicalRpcService::Invoke(const std::string &func, std::shared_ptr<Rpc::Packet> message)
    {
        if (!this->IsStartService() || !this->mIsHandle)
        {
            LOG_ERROR(this->GetName() << " is not start");
            return XCode::CallServiceNotFound;
        }
        std::shared_ptr<ServiceMethod> serviceMethod = this->mMethodRegister.GetMethod(func);
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

	bool PhysicalRpcService::Start()
	{
		if(!this->OnStart())
		{
			return false;
		}
		LuaScriptComponent* luaScriptComponent = this->GetComponent<LuaScriptComponent>();
		if(luaScriptComponent == nullptr)
		{
			return true;
		}
		const std::string & name = this->GetName();
		Lua::LuaModule* luaModule = luaScriptComponent->LoadModule(name);
		if (luaModule == nullptr)
		{
			return true;
		}
		return luaModule->Start();
	}

	bool PhysicalRpcService::Init()
	{
		if(!this->OnInit())
		{
			return false;
		}
		this->mIsHandle = true;
		LuaScriptComponent* luaScriptComponent = this->GetComponent<LuaScriptComponent>();
		if (luaScriptComponent != nullptr)
		{
			const std::string& name = this->GetName();
			Lua::LuaModule* luaModule = luaScriptComponent->LoadModule(name);
			if (luaModule != nullptr)
			{
				if(!this->LoadFromLua())
				{
					LOG_ERROR("lua rpc module [" << name << "] LoadFromLua");
					return false;
				}
			}
			const RpcServiceConfig* rpcServiceConfig =
				RpcConfig::Inst()->GetConfig(this->GetName());
			std::vector<const RpcMethodConfig*> methodConfigs;
			rpcServiceConfig->GetMethodConfigs(methodConfigs);
			for (const RpcMethodConfig* config : methodConfigs)
			{
				if (this->mMethodRegister.GetMethod(config->Method) == nullptr)
				{
					LOG_ERROR("not register " << config->FullName);
					return false;
				}
			}
		}
		return true;
	}

    void PhysicalRpcService::OnRecord(Json::Writer&document)
    {    
        document.Add("sum").Add(this->mSumCount);
        document.Add("wait").Add(this->mWaitCount);
    }

    bool PhysicalRpcService::LoadFromLua()
	{
		LuaScriptComponent* luaScriptComponent = this->GetComponent<LuaScriptComponent>();       
        if (luaScriptComponent == nullptr)
        {
            return true;
        }
		const std::string & name = this->GetName();
		Lua::LuaModule* luaModule = luaScriptComponent->GetModule(name);
		if (luaModule == nullptr)
		{
			return true;
		}
        std::vector<const RpcMethodConfig*> methodConfigs;
		const RpcServiceConfig* rpcServiceConfig = RpcConfig::Inst()->GetConfig(name);
        if (rpcServiceConfig == nullptr)
        {
			luaScriptComponent->UnloadModule(name);
            return false;
        }
		rpcServiceConfig->GetMethodConfigs(methodConfigs);
		for (const RpcMethodConfig* methodConfig : methodConfigs)
		{
            std::shared_ptr<ServiceMethod> serviceMethod =  
                this->mMethodRegister.GetMethod(methodConfig->Method);
            if (serviceMethod != nullptr && serviceMethod->IsLuaMethod())
            {
                continue;
            }
			if(luaModule->GetFunction(methodConfig->Method))
			{
				std::shared_ptr<LuaServiceMethod> luaServiceMethod
					= std::make_shared<LuaServiceMethod>(methodConfig);
				this->mMethodRegister.AddMethod(luaServiceMethod);
				LOG_WARN(methodConfig->FullName << " use lua method");
			}
		}
		return true;
	}

	bool PhysicalRpcService::Close()
    {
		if (!this->IsStartService())
        {
            return false;
        }
        this->OnClose();
		this->mMethodRegister.ClearLuaMethods();
		return true;
    }
}