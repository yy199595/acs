//
// Created by yjz on 2022/1/23.
//
#include"LocalHttpService.h"
#include"Component/LuaScriptComponent.h"
#include"Method/LuaHttpServiceMethod.h"
namespace Sentry
{

	XCode LocalHttpService::Invoke(const std::string& name,
		std::shared_ptr<Http::Request> request, std::shared_ptr<Http::Response> response)
	{
		std::shared_ptr<HttpServiceMethod> method = this->mServiceRegister->GetMethod(name);
		if(method == nullptr)
		{
			return XCode::CallServiceNotFound;
		}
		return method->Invoke(*request, *response);
	}

    bool LocalHttpService::LateAwake()
    {
        LuaScriptComponent* luaComponent = this->GetComponent<LuaScriptComponent>();
        Lua::LuaModule * luaModule = luaComponent->LoadModule(this->GetName());
        return luaModule == nullptr || luaModule->Awake();
    }

	bool LocalHttpService::Start()
    {
        this->mServiceRegister = std::make_shared<HttpServiceRegister>(this);
        LuaScriptComponent * luaComponent = this->GetComponent<LuaScriptComponent>();
        const HttpServiceConfig * httpServiceConfig = HttpConfig::Inst()->GetConfig(this->GetName());
        if (!this->OnStartService(*this->mServiceRegister))
        {
            return false;
        }
        std::vector<const HttpMethodConfig*> methodConfigs;
        httpServiceConfig->GetMethodConfigs(methodConfigs);
        Lua::LuaModule* luaModule = luaComponent->GetModule(this->GetName());
        if (luaModule != nullptr)
        {
            for (const HttpMethodConfig* config : methodConfigs)
            {
                if (luaModule->GetFunction(config->Method))
                {                   
                    if (!this->mServiceRegister->AddMethod(std::make_shared<LuaHttpServiceMethod>(config)))
                    {
                        return false;
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
        std::move(this->mServiceRegister);
        if (!this->OnCloseService())
        {
            return false;
        }
        LuaScriptComponent* luaComponent = this->GetComponent<LuaScriptComponent>();
        Lua::LuaModule* luaModule = luaComponent->GetModule(this->GetName());
        return (luaModule == nullptr || luaModule->Close());
	}
}