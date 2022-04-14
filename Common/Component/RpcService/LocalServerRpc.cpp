//
// Created by mac on 2022/4/6.
//

#include"LocalServerRpc.h"
#include"App/App.h"
#include"Global/RpcConfig.h"
#include"Method/LuaServiceMethod.h"
#include"Component/Lua/LuaScriptComponent.h"

namespace Sentry
{

	bool ServiceMethodRegister::AddMethod(std::shared_ptr<ServiceMethod> method)
	{
		const std::string & name = method->GetName();
		const RpcConfig & rpcConfig = App::Get()->GetRpcConfig();
		if (!rpcConfig.HasServiceMethod(this->mService, name))
		{
			LOG_FATAL(this->mService << "." << name << " add failure");
			return false;
		}

		if (method->IsLuaMethod())
		{
			auto iter = this->mLuaMethodMap.find(name);
			if (iter != this->mLuaMethodMap.end())
			{
				this->mLuaMethodMap.erase(iter);
			}
			this->mLuaMethodMap.emplace(name, method);
			LOG_DEBUG("add new lua service method [" << this->mService << '.' << name << "]");
			return true;
		}

		auto iter = this->mMethodMap.find(name);
		if (iter != this->mMethodMap.end())
		{
			LOG_FATAL(this->mService << "." << name << " add failure");
			return false;
		}
		this->mMethodMap.emplace(name, method);
		LOG_DEBUG("add new c++ service method [" << this->mService <<'.' << name << ']');
		return true;
	}

	std::shared_ptr<ServiceMethod> ServiceMethodRegister::GetMethod(const string& name)
	{
		auto iter = this->mLuaMethodMap.find(name);
		if(iter != this->mLuaMethodMap.end())
		{
			return iter->second;
		}
		auto iter1 = this->mMethodMap.find(name);
		if(iter1 != this->mMethodMap.end())
		{
			return iter1->second;
		}
		return nullptr;
	}

	bool ServiceMethodRegister::LoadLuaMethod(lua_State* lua)
	{
		this->mLuaMethodMap.clear();
		const char* tab = this->mService.c_str();
		auto iter = this->mMethodMap.begin();
		for (; iter != this->mMethodMap.end(); iter++)
		{
			const std::string& name = iter->first;
			if (Lua::Function::Get(lua, tab, name.c_str()))
			{
				std::shared_ptr<LuaServiceMethod> luaServiceMethod
					= std::make_shared<LuaServiceMethod>(this->mService, name, lua);
				this->mLuaMethodMap.emplace(name, luaServiceMethod);
			}
		}
		return true;
	}

	ServiceMethodRegister::ServiceMethodRegister(const string& service, void * o)
		: mService(service), mObj(o)
	{

	}
}

namespace Sentry
{
	std::shared_ptr<com::Rpc_Response> LocalServerRpc::Invoke(const string& method, std::shared_ptr<com::Rpc_Request> request)
	{
		assert(this->IsStartService());
		auto serviceMethod = this->mMethodRegister->GetMethod(method);
		if (serviceMethod == nullptr)
		{
			return nullptr;
		}
		std::shared_ptr<com::Rpc_Response> response(new com::Rpc_Response());

		response->set_rpc_id(request->rpc_id());
		response->set_user_id(request->user_id());
		try
		{
			XCode code = serviceMethod->Invoke(*request, *response);
			if (request->rpc_id() == 0)
			{
				return nullptr;
			}
			response->set_code((int)code);
		}
		catch (std::logic_error& logic_error)
		{
			response->set_code((int)XCode::ThrowError);
			response->set_error_str(logic_error.what());
		}
		return response;
	}

	bool LocalServerRpc::LoadService()
	{
		if(this->mMethodRegister == nullptr)
		{
			this->mMethodRegister = std::make_shared<ServiceMethodRegister>(this->GetName(), this);
			if(!this->OnInitService(*this->mMethodRegister))
			{
				return false;
			}
		}
		LuaScriptComponent * luaScriptComponent = this->GetComponent<LuaScriptComponent>();
		if(luaScriptComponent != nullptr)
		{
			lua_State * lua = luaScriptComponent->GetLuaEnv();
			return this->mMethodRegister->LoadLuaMethod(lua);
		}
		return true;
	}
}