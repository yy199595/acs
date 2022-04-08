//
// Created by mac on 2022/4/6.
//

#include"LocalServerRpc.h"
#include"App/App.h"
#include"Global/RpcConfig.h"

namespace Sentry
{
	bool LocalServerRpc::AddMethod(std::shared_ptr<ServiceMethod> method)
	{
		const RpcConfig & rpcConfig = App::Get()->GetRpcConfig();
		const std::string & name = method->GetName();
		const std::string & service = this->GetName();
		if (!rpcConfig.HasServiceMethod(service, name))
		{
			LOG_FATAL(service, '.', name, " add failure");
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
			LOG_DEBUG("add new lua service method [{0}.{1}]", service, name);
			return true;
		}

		auto iter = this->mMethodMap.find(name);
		if (iter != this->mMethodMap.end())
		{
			LOG_FATAL("{0}.{1} add failure", service, name);
			return false;
		}
		this->mMethodMap.emplace(name, method);
		LOG_DEBUG("add new c++ service method [{0}.{1}]", service, name);
		return true;
	}

	std::shared_ptr<ServiceMethod> LocalServerRpc::GetMethod(const string& name)
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
}

namespace Sentry
{
	std::shared_ptr<com::Rpc_Response> LocalServerRpc::Invoke(const string& method, std::shared_ptr<com::Rpc_Request> request)
	{
		auto serviceMethod = this->GetMethod(method);
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

	void LocalServerRpc::OnLuaRegister(lua_State* lua)
	{
		auto iter = this->mMethodMap.begin();
		for(; iter != this->mMethodMap.end(); iter++)
		{
			const std::string & name = iter->first;
		}
	}
}