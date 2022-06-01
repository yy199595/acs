//
// Created by mac on 2022/4/6.
//

#include <Method/MethodRegister.h>

#include"LocalServiceComponent.h"
#include"App/App.h"
#include"Global/ServiceConfig.h"
#include"Method/LuaServiceMethod.h"
#include"Component/Rpc/RpcClientComponent.h"
#include"Component/Lua/LuaScriptComponent.h"

namespace Sentry
{
	XCode LocalRpcService::Invoke(const std::string& func, std::shared_ptr<com::Rpc::Request> request,
	    std::shared_ptr<com::Rpc::Response> response)
	{
		if (!this->IsStartService())
		{
			LOG_ERROR(this->GetName() << " is not start");
			return XCode::CallServiceNotFound;
		}

		std::shared_ptr<ServiceMethod> serviceMethod = this->mMethodRegister->GetMethod(func);
		if (serviceMethod == nullptr)
		{
			LOG_ERROR("not find [" << this->GetName() << "." << func << "]");
			return XCode::CallServiceNotFound;
		}
		return serviceMethod->Invoke(*request, *response);
	}

	bool LocalRpcService::StartService()
	{
		this->mMethodRegister = std::make_shared<ServiceMethodRegister>(this);
		if (!this->OnStartService(*this->mMethodRegister))
		{
			return false;
		}
		LuaScriptComponent* luaScriptComponent = this->GetComponent<LuaScriptComponent>();
		if (luaScriptComponent != nullptr)
		{
			lua_State* lua = luaScriptComponent->GetLuaEnv();
			return this->mMethodRegister->LoadLuaMethod(lua);
		}
		return true;
	}

	bool LocalRpcService::CloseService()
	{
		if(this->IsStartService())
		{
			std::move(this->mMethodRegister);
			return true;
		}
		return false;
	}
}