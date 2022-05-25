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
	XCode LocalRpcServiceBase::Invoke(const std::string& func, std::shared_ptr<com::Rpc::Request> request,
	    std::shared_ptr<com::Rpc::Response> response)
	{
		assert(this->IsStartService());
		response->set_rpc_id(request->rpc_id());
		response->set_user_id(request->user_id());

		std::shared_ptr<ServiceMethod> serviceMethod = this->mMethodRegister->GetMethod(func);
		if (serviceMethod == nullptr)
		{
			response->set_code((int)XCode::CallServiceNotFound);
			LOG_ERROR("not find [" << this->GetName() << "." << func << "]");
			return XCode::CallServiceNotFound;
		}

		try
		{
			return serviceMethod->Invoke(*request, *response);
		}
		catch (std::logic_error& logic_error)
		{
			response->set_error_str(logic_error.what());
			return XCode::ThrowError;
		}
	}

	bool LocalRpcServiceBase::StartService()
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

	bool LocalRpcServiceBase::CloseService()
	{
		if(this->IsStartService())
		{
			std::move(this->mMethodRegister);
			return true;
		}
		return false;
	}
}