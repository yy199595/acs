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
	bool LocalHttpService::Start()
    {
        this->mServiceRegister = std::make_shared<HttpServiceRegister>(this);
        LuaScriptComponent * luaComponent = this->GetComponent<LuaScriptComponent>();
        const HttpServiceConfig * httpServiceConfig = HttpConfig::Inst()->GetConfig(this->GetName());
        if (!this->OnStartService(*this->mServiceRegister))
        {
            return false;
        }

        std::vector<const HttpMethodConfig *> methodConfigs;
        httpServiceConfig->GetMethodConfigs(methodConfigs);
        for(const HttpMethodConfig * config : methodConfigs)
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

        for(const HttpMethodConfig * methodConfig : methodConfigs)
        {
            if(this->mServiceRegister->GetMethod(methodConfig->Method) == nullptr)
            {
                LOG_ERROR("not register method " << methodConfig->Service << "." << methodConfig->Method);
                return false;
            }
        }
        return true;
    }

	bool LocalHttpService::Close()
	{
        this->mServiceRegister->Clear();
        std::move(this->mServiceRegister);
		return this->OnCloseService();
	}
}