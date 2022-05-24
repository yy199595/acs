//
// Created by yjz on 2022/5/23.
//

#include"LuaService.h"
#include"App/App.h"
#include"Async/RpcTask/RpcTaskSource.h"
using namespace Sentry;
namespace Lua
{
	int Service::Call(lua_State* lua)
	{
		if (!lua_isuserdata(lua, 1))
		{
			luaL_error(lua, "call paremeter first is service");
			return 0;
		}
		std::string address;
		std::shared_ptr<com::Rpc::Request> request(new com::Rpc::Request());
		ServiceCallComponent* callComponent = (ServiceCallComponent*)lua_touserdata(lua, 1);
		if (lua_isinteger(lua, 2)) //userId
		{
			long long userId = lua_tointeger(lua, 2);
			if (!callComponent->GetEntityAddress(userId, address))
			{
				lua_pushinteger(lua, (int)XCode::NotFindUser);
				luaL_error(lua, "not find user service address %lld", userId);
				return 1;
			}
			request->set_user_id(userId);
		}
		else if (lua_isstring(lua, 2)) //address
		{
			address.append(lua_tostring(lua, 2));
		}
		if (!lua_isstring(lua, 3))
		{
			lua_pushinteger(lua, (int)XCode::CallArgsError);
			luaL_error(lua, "not find service function");
			return 1;
		}
		std::string func = lua_tostring(lua, 3);
		const ServiceConfig& serviceConfig = App::Get()->GetServiceConfig();
		std::string fullName = fmt::format("{0}.{1}", callComponent->GetName(), func);
		const RpcInterfaceConfig* rpcInterfaceConfig = serviceConfig.GetInterfaceConfig(fullName);
		if (rpcInterfaceConfig == nullptr)
		{
			lua_pushinteger(lua, (int)XCode::NotFoundRpcConfig);
			return 1;
		}
		if (!rpcInterfaceConfig->Request.empty())
		{
			if (!lua_isstring(lua, 4))
			{
				lua_pushinteger(lua, (int)XCode::CallArgsError);
				luaL_error(lua, "call %s request json should %s", fullName.c_str(),
						rpcInterfaceConfig->Request.c_str());
				return 1;
			}
		}
		lua_pushthread(lua);
		std::shared_ptr<LuaRpcTaskSource> luaRpcTaskSource(new LuaRpcTaskSource(lua));
		if(!luaRpcTaskSource->Yield())
		{
			luaL_error(lua, "not lua coroutine context");
			return 0;
		}
		TaskComponent * taskComponent = App::Get()->GetTaskComponent();
		taskComponent->Start([lua, request, address, luaRpcTaskSource, callComponent]()
		{
			std::shared_ptr<com::Rpc::Response> response(new com::Rpc::Response());
			XCode code = callComponent->Call(address, request, response);
			luaRpcTaskSource->SetResult(code, *response);
		});
		return 0;
	}

	int Service::AllotAddress(lua_State* lua)
	{
		return 0;
	}
}