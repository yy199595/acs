//
// Created by yjz on 2022/5/23.
//

#include"LuaService.h"
#include"App/App.h"
#include"Util/StringHelper.h"
#include"Pool/MessagePool.h"
#include"Script/Extension/Json/Json.h"
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
		long long userId = 0;
		std::shared_ptr<com::Rpc::Request> request(new com::Rpc::Request());
		ServiceCallComponent* callComponent = UserDataParameter::Read<ServiceCallComponent*>(lua, 1);
		if (lua_isinteger(lua, 2)) //userId
		{
			userId = lua_tointeger(lua, 2);
			request->set_user_id(userId);
		}
		else if (lua_isstring(lua, 2)) //address
		{
			address.append(lua_tostring(lua, 2));
		}
		if (address.empty() && userId == 0)
		{
			luaL_error(lua, "first muset userId or address");
			return 0;
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
			if (!lua_istable(lua, 4))
			{
				lua_pushinteger(lua, (int)XCode::CallArgsError);
				luaL_error(lua, "call %s request tabe should %s", fullName.c_str(),
						rpcInterfaceConfig->Request.c_str());
				return 1;
			}
			lua_pushcfunction(lua, Lua::Json::Encode);
			lua_pushvalue(lua, 4);
			if(lua_pcall(lua, 1, 1, 0) != 0)
			{
				luaL_error(lua, lua_tostring(lua, -1));
				return 0;
			}
			size_t size = 0;
			const char * json = lua_tolstring(lua, -1, &size);
			const std::string & name = rpcInterfaceConfig->Request;
			std::shared_ptr<Message> message = Helper::Proto::NewByJson(name, json, size);
			if(message == nullptr)
			{
				luaL_error(lua, "lua table to %s failure", name.c_str());
				return 0;
			}
			request->mutable_data()->PackFrom(*message);
		}
		std::shared_ptr<Message> response;
		if(!rpcInterfaceConfig->Response.empty())
		{
			response = Helper::Proto::New(rpcInterfaceConfig->Response);
		}
		lua_pushthread(lua);
		request->set_method_id(rpcInterfaceConfig->InterfaceId);
		TaskComponent* taskComponent = App::Get()->GetTaskComponent();
		std::shared_ptr<LuaRpcTaskSource> luaRpcTaskSource(new LuaRpcTaskSource(lua));
		taskComponent->Start([lua, request, response, address, luaRpcTaskSource, callComponent, userId]()
		{
			XCode code = address.empty() ? callComponent->Call(userId, request, response)
										 : callComponent->Call(address, request, response);
			luaRpcTaskSource->SetResult(code, response);
		});
		return luaRpcTaskSource->Yield();
	}

	int Service::AllotAddress(lua_State* lua)
	{
		return 0;
	}
}