//
// Created by yjz on 2022/5/23.
//

#include"LuaService.h"
#include"App/App.h"
#include"Util/StringHelper.h"
#include"Script/Extension/Json/Json.h"
#include"Async/Lua/LuaWaitTaskSource.h"
#include"Component/Scene/MessageComponent.h"
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
		ServiceComponent* callComponent = UserDataParameter::Read<ServiceComponent*>(lua, 1);
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
		std::string method = lua_tostring(lua, 3);
		const RpcServiceConfig & rpcServiceConfig = callComponent->GetServiceConfig();
		const RpcInterfaceConfig* rpcInterfaceConfig = rpcServiceConfig.GetConfig(method);
		if (rpcInterfaceConfig == nullptr)
		{
			lua_pushinteger(lua, (int)XCode::NotFoundRpcConfig);
			return 0;
		}
		if (!rpcInterfaceConfig->Request.empty())
		{
			if (!lua_isuserdata(lua, 4))
			{
				lua_pushinteger(lua, (int)XCode::CallArgsError);
				luaL_error(lua, "call %s request message should %s", rpcInterfaceConfig->FullName.c_str(),
						rpcInterfaceConfig->Request.c_str());
				return 0;
			}
			std::shared_ptr<Message> message = UserDataParameter::Read<std::shared_ptr<Message>>(lua, 4);
			if(message == nullptr)
			{
				return 0;
			}
			request->mutable_data()->PackFrom(*message);
		}
		std::shared_ptr<Message> response;
		if(!rpcInterfaceConfig->Response.empty())
		{
			MessageComponent * messageComponent = App::Get()->GetComponent<MessageComponent>();
			response = messageComponent->New(rpcInterfaceConfig->Response);
		}
		lua_pushthread(lua);
		request->set_func(rpcInterfaceConfig->FullName);
		TaskComponent* taskComponent = App::Get()->GetTaskComponent();
		std::shared_ptr<LuaWaitTaskSource> luaRpcTaskSource(new LuaWaitTaskSource(lua));
		taskComponent->Start([lua, request, response, address, luaRpcTaskSource, callComponent, userId]()
		{
			XCode code = address.empty() ? callComponent->Call(userId, request, response)
										 : callComponent->Call(address, request, response);
			luaRpcTaskSource->SetResult(code, response);
		});
		return luaRpcTaskSource->Await();
	}

	int Service::AllotAddress(lua_State* lua)
	{
		return 0;
	}
}