//
// Created by mac on 2022/4/6.
//

#include"LocalServerRpc.h"
#include"App/App.h"
#include"Global/RpcConfig.h"
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