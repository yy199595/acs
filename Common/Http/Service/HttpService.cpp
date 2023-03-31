//
// Created by yjz on 2022/1/23.
//
#include"HttpService.h"
#include"Script/Module/LuaModule.h"
#include"Script/Component/LuaScriptComponent.h"
#include"Http/Method/LuaHttpServiceMethod.h"
namespace Sentry
{

    HttpService::HttpService()
		: mServiceRegister(this)
    {
		this->mLuaComponent = nullptr;
    }

    bool HttpService::LateAwake()
    {
		this->mLuaComponent = this->GetComponent<LuaScriptComponent>();
		return true;
    }

	bool HttpService::Init()
    {
		const std::string & name = this->GetName();
		const HttpConfig * config = HttpConfig::Inst();
        const HttpServiceConfig * httpServiceConfig = config->GetConfig(name);
        if(httpServiceConfig == nullptr)
		{
			LOG_ERROR("not find http service : " << name);
			return false;
		}
		if(this->mLuaComponent != nullptr && this->IsStartService())
		{
			this->mLuaComponent->LoadModule(name);
		}
		if(!this->OnInit() || !this->LoadFromLua())
		{
			return false;
		}
		std::vector<const HttpMethodConfig*> methodConfigs;
		httpServiceConfig->GetMethodConfigs(methodConfigs);
        for(const HttpMethodConfig * methodConfig : methodConfigs)
        {
            if(this->mServiceRegister.GetMethod(methodConfig->Method) == nullptr)
            {
                LOG_ERROR("not register method " << methodConfig->Service << "." << methodConfig->Method);
                return false;
            }
        }
        return true;
    }

	bool HttpService::LoadFromLua()
	{
		LuaScriptComponent* luaComponent = this->GetComponent<LuaScriptComponent>();
		if (luaComponent == nullptr || !this->IsStartService())
		{
			return true;
		}
		const std::string& name = this->GetName();
		Lua::LuaModule* luaModule = luaComponent->GetModule(name);
		if (luaModule != nullptr)
		{
			const HttpConfig* config = HttpConfig::Inst();
			std::vector<const HttpMethodConfig*> methodConfigs;
			config->GetConfig(name)->GetMethodConfigs(methodConfigs);

			HttpServiceRegister& httpRegister = this->GetRegister();
			for (const HttpMethodConfig* config : methodConfigs)
			{
				if (luaModule->GetFunction(config->Method))
				{
					httpRegister.AddMethod(std::make_shared<LuaHttpServiceMethod>(config));
				}
			}
		}
		return true;
	}

	bool HttpService::Start()
	{
		if(!this->OnStart())
		{
			return false;
		}
		const std::string & name = this->GetName();
		if(this->mLuaComponent != nullptr)
		{
			Lua::LuaModule* luaModule = this->mLuaComponent->GetModule(name);
			return luaModule == nullptr || luaModule->Start();
		}
		return true;
	}

	bool HttpService::Close()
	{
        this->mServiceRegister.Clear();
        if (!this->OnClose())
        {
            return false;
        }

		const std::string & name = this->GetName();
		if(this->mLuaComponent != nullptr)
		{
			Lua::LuaModule* luaModule = this->mLuaComponent->GetModule(name);
			return luaModule == nullptr || luaModule->Close();
		}
		return true;
	}
}