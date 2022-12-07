//
// Created by yjz on 2022/1/23.
//
#include"LocalHttpService.h"
#include"Component/LuaScriptComponent.h"
#include"Method/LuaHttpServiceMethod.h"
namespace Sentry
{

    LocalHttpService::LocalHttpService()
    {
        this->mSumCount = 0;
        this->mWaitCount = 0;
    }

	XCode LocalHttpService::Invoke(const std::string& name,
		std::shared_ptr<Http::Request> request, std::shared_ptr<Http::Response> response)
	{
		std::shared_ptr<HttpServiceMethod> method = this->mServiceRegister->GetMethod(name);
		if(method == nullptr)
		{
			return XCode::CallServiceNotFound;
		}
        this->mSumCount++;
        this->mWaitCount++;
		XCode code = method->Invoke(*request, *response);
        {
            this->mWaitCount--;
        }
        return code;
	}

    bool LocalHttpService::LateAwake()
    {
        LuaScriptComponent* luaComponent = this->GetComponent<LuaScriptComponent>();
        Lua::LuaModule * luaModule = luaComponent->LoadModule(this->GetName());
        return luaModule == nullptr || luaModule->Awake();
    }

    bool LocalHttpService::LoadFromLua()
    {
        LuaScriptComponent* luaComponent = this->GetComponent<LuaScriptComponent>();
        const HttpServiceConfig* httpServiceConfig = HttpConfig::Inst()->GetConfig(this->GetName());
        LOG_CHECK_RET_FALSE(luaComponent != nullptr && httpServiceConfig != nullptr);
        std::vector<const HttpMethodConfig*> methodConfigs;
        httpServiceConfig->GetMethodConfigs(methodConfigs);
        Lua::LuaModule* luaModule = luaComponent->GetModule(this->GetName());
        if (luaModule != nullptr)
        {
            for (const HttpMethodConfig* config : methodConfigs)
            {
                std::shared_ptr<HttpServiceMethod> serviceMethod = 
                    this->mServiceRegister->GetMethod(config->Method);
                if (serviceMethod != nullptr && serviceMethod->IsLuaMethod())
                {
                    continue;
                }
                if (luaModule->GetFunction(config->Method))
                {
                    this->mServiceRegister->AddMethod(std::make_shared<LuaHttpServiceMethod>(config));                  
                }
            }
        }
        return true;
    }

    void LocalHttpService::OnRecord(Json::Writer&document)
    {
        document.Add("sum").Add(this->mSumCount);
        document.Add("wait").Add(this->mWaitCount);
    }

	bool LocalHttpService::Start()
    {
        this->mServiceRegister = std::make_shared<HttpServiceRegister>(this);
        LuaScriptComponent * luaComponent = this->GetComponent<LuaScriptComponent>();
        const HttpServiceConfig * httpServiceConfig = HttpConfig::Inst()->GetConfig(this->GetName());
        if(httpServiceConfig == nullptr)
        {
            LOG_ERROR("not find http service : " << this->GetName());
            return false;
        }
        if (!this->OnStartService(*this->mServiceRegister))
        {
            return false;
        }
        Lua::LuaModule* luaModule = nullptr;
        std::vector<const HttpMethodConfig*> methodConfigs;
        httpServiceConfig->GetMethodConfigs(methodConfigs);
        if (luaComponent != nullptr)
        {
            luaModule = luaComponent->GetModule(this->GetName());
            if (luaModule != nullptr)
            {
                for (const HttpMethodConfig* config : methodConfigs)
                {
                    if (luaModule->GetFunction(config->Method))
                    {
                        this->mServiceRegister->AddMethod(std::make_shared<LuaHttpServiceMethod>(config));                      
                    }
                }
            }
        }

        for(const HttpMethodConfig * methodConfig : methodConfigs)
        {
            if(this->mServiceRegister->GetMethod(methodConfig->Method) == nullptr)
            {
                LOG_ERROR("not register method " << methodConfig->Service << "." << methodConfig->Method);
                return false;
            }
        }
        return luaModule == nullptr || luaModule->Start();
    }

	bool LocalHttpService::Close()
	{
        this->mServiceRegister->Clear();
		this->mServiceRegister = nullptr;
        if (!this->OnCloseService())
        {
            return false;
        }
        LuaScriptComponent* luaComponent = this->GetComponent<LuaScriptComponent>();
        Lua::LuaModule* luaModule = luaComponent->GetModule(this->GetName());
        return (luaModule == nullptr || luaModule->Close());
	}
}