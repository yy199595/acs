//
// Created by yjz on 2022/1/23.
//
#include"LocalHttpService.h"
#include"App/App.h"
#include"spdlog/fmt/fmt.h"
#include"Network/Http/HttpAsyncRequest.h"
#include"Component/Lua/LuaScriptComponent.h"
#include"Method/LuaHttpServiceMethod.h"
namespace Sentry
{
	LocalHttpService::LocalHttpService()
	{
		this->mConfig = nullptr;
	}

	XCode LocalHttpService::Invoke(const std::string& name,
		std::shared_ptr<HttpHandlerRequest> request, std::shared_ptr<HttpHandlerResponse> response)
	{
		std::shared_ptr<HttpServiceMethod> method = this->mServiceRegister->GetMethod(name);
		if(method == nullptr)
		{
			return XCode::CallServiceNotFound;
		}
		return method->Invoke(*request, *response);
	}
	bool LocalHttpService::StartNewService()
    {
        this->mServiceRegister = std::make_shared<HttpServiceRegister>(this);
        if (!this->OnStartService(*this->mServiceRegister))
        {
            return false;
        }

        std::vector<const HttpInterfaceConfig *> configs;
        this->GetServiceConfig().GetConfigs(configs);
        LuaScriptComponent * luaComponent = this->GetComponent<LuaScriptComponent>();
        for(const HttpInterfaceConfig * config : configs)
        {
            const char * tab = config->Service.c_str();
            const char * method = config->Method.c_str();
            lua_State * luaEnv = luaComponent->GetLuaEnv();
            if(Lua::Function::Get(luaEnv,tab, method))
            {
                std::shared_ptr<LuaHttpServiceMethod> luaHttpServiceMethod =
                        std::make_shared<LuaHttpServiceMethod>(config, luaEnv);
                if(!this->mServiceRegister->AddMethod(luaHttpServiceMethod))
                {
                    return false;
                }
            }
        }

        for(const HttpInterfaceConfig * config : configs)
        {
            if(this->mServiceRegister->GetMethod(config->Method) == nullptr)
            {
                LOG_ERROR("not register method " << config->Service << "." << config->Method);
                return false;
            }
        }
        return true;
    }

	bool LocalHttpService::CloseService()
	{
        this->mServiceRegister->Clear();
		return this->OnCloseService();
	}
	bool LocalHttpService::LoadConfig(const rapidjson::Value& json)
	{
		if(this->mConfig == nullptr)
		{
			this->mConfig = new HttpServiceConfig(this->GetName());
		}
		return this->mConfig->OnLoadConfig(json);
	}
}