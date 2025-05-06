//
// Created by 64658 on 2025/4/10.
//

#include "LuaRouter.h"
#include "XCode/XCode.h"
#include "Entity/Actor/App.h"
#include "Router/Component/RouterComponent.h"
namespace acs
{
	int LuaRouter::Send(lua_State* L)
	{
		int code = XCode::Ok;
		int id = (int)luaL_checkinteger(L, 1);
		std::unique_ptr<rpc::Message> message;
		const std::string func(luaL_checkstring(L, 2));
		do
		{
			static RouterComponent * routerComponent = nullptr;
			if(routerComponent == nullptr)
			{
				routerComponent = acs::App::Get<RouterComponent>();
			}
			code = acs::App::Inst()->MakeMessage(L, 3, func, message);
			if(code != XCode::Ok)
			{
				break;
			}
			message->SetSockId(id);
			code = routerComponent->Send(id, std::move(message));
		}
		while(false);
		lua_pushinteger(L, code);
		return 1;
	}

	int LuaRouter::Call(lua_State* L)
	{
		lua_pushthread(L);
		int code = XCode::Ok;
		int id = (int)luaL_checkinteger(L, 1);
		std::unique_ptr<rpc::Message> message;
		const std::string func(luaL_checkstring(L, 2));
		do
		{
			static RouterComponent * routerComponent = nullptr;
			if(routerComponent == nullptr)
			{
				routerComponent = acs::App::Get<RouterComponent>();
			}
			code = acs::App::Inst()->MakeMessage(L, 3, func, message);
			if(code != XCode::Ok)
			{
				break;
			}
			message->SetSockId(id);
			return routerComponent->LuaCall(L, id, std::move(message));
		}
		while(false);
		lua_pushinteger(L, code);
		return 1;
	}
}