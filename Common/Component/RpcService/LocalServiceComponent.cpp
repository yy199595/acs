//
// Created by mac on 2022/4/6.
//

#include"LocalServiceComponent.h"
#include"App/App.h"
#include"Global/ServiceConfig.h"
#include"Method/LuaServiceMethod.h"
#include"Component/Rpc/RpcClientComponent.h"
#include"Component/Lua/LuaScriptComponent.h"
#include"Component/Service/UserInfoSyncService.h"

namespace Sentry
{
	bool ServiceMethodRegister::AddMethod(std::shared_ptr<ServiceMethod> method)
	{
		const std::string & name = method->GetName();
		const ServiceConfig & rpcConfig = App::Get()->GetServiceConfig();
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
	void LocalServiceComponent::Awake()
	{
		this->mIndex = 0;
		this->mAddressList.clear();
	}
	XCode LocalServiceComponent::Invoke(const std::string& func, std::shared_ptr<Rpc_Request> request,
	    std::shared_ptr<Rpc_Response> response)
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

	bool LocalServiceComponent::LoadService()
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

	bool LocalServiceComponent::AddEntity(long long id, const std::string & address)
	{
		this->mUserAddressMap[id] = address;
		LOG_INFO(this->GetName() << " add " << id << " address = " << address);
		return true;
	}

	bool LocalServiceComponent::DelEntity(long long id)
	{
		auto iter = this->mUserAddressMap.find(id);
		if (iter != this->mUserAddressMap.end())
		{
			this->mUserAddressMap.erase(iter);
			LOG_INFO(this->GetName() << " del " << id);
			return true;
		}
		return true;
	}

	bool LocalServiceComponent::AllotAddress(string& address)
	{
		if(!this->mAddressList.empty())
		{
			address = this->mAddressList[this->mIndex++];
			if(this->mIndex >= this->mAddressList.size())
			{
				this->mIndex = 0;
			}
			return true;
		}
		return false;
	}

	void LocalServiceComponent::OnDelAddress(const string& address)
	{
		auto iter = std::find_if(this->mAddressList.begin(), this->mAddressList.end(), address);
		if(iter != this->mAddressList.end())
		{
			this->mAddressList.erase(iter);
			LOG_WARN("{0} delete address " << this->GetName() << '.' << address);
		}
	}
	void LocalServiceComponent::OnAddAddress(const string& address)
	{
		assert(!address.empty());
		auto iter = std::find_if(this->mAddressList.begin(), this->mAddressList.end(), address);
		if(iter == this->mAddressList.end())
		{
			this->mAddressList.emplace_back(address);
			LOG_ERROR(this->GetName() << " add address " << address);
		}
	}
	bool LocalServiceComponent::GetEntityAddress(long long int id, string& address)
	{
		auto iter = this->mUserAddressMap.find(id);
		if(iter != this->mUserAddressMap.end())
		{
			address = iter->second;
			return true;
		}
		return false;
	}

}