//
// Created by leyi on 2023/11/8.
//
#include<string>
#include"LuaClient.h"
#include"XCode/XCode.h"
#include"Rpc/Client/Message.h"
#include"Lua/Engine/Define.h"
#include"Entity/Actor/App.h"
#include"Router/Component/RouterComponent.h"
#include"Client/Component/ClientComponent.h"

namespace acs
{
	int LuaClient::Send(lua_State* l)
	{
		std::unique_ptr<rpc::Packet> message;
		int sessionId = (int)luaL_checkinteger(l, 1);
		const std::string func = luaL_checkstring(l, 2);
		if(App::Inst()->MakeMessage(l, 3, func, message) != XCode::Ok)
		{
			lua_pushinteger(l, XCode::Failure);
			return 1;
		}
		RouterComponent * router = App::Get<RouterComponent>();
		if(router == nullptr)
		{
			lua_pushinteger(l, XCode::NetWorkError);
			return 1;
		}
		message->SetNet(rpc::Net::Client);
		lua_pushinteger(l, router->Send(sessionId, std::move(message)));
		return 1;
	}

	int LuaClient::Call(lua_State* l)
	{
        lua_pushthread(l);
		std::unique_ptr<rpc::Packet> message;
		int sessionId = (int)luaL_checkinteger(l, 1);
		const std::string func = luaL_checkstring(l, 2);
		if(App::Inst()->MakeMessage(l, 3, func, message) != XCode::Ok)
		{
			lua_pushinteger(l, XCode::Failure);
			return 1;
		}
		RouterComponent * router = App::Get<RouterComponent>();
		if(router == nullptr)
		{
			lua_pushinteger(l, XCode::NetWorkError);
			return 1;
		}
		message->SetNet(rpc::Net::Client);
		return router->LuaCall(l, sessionId, std::move(message));
	}

	int LuaClient::Connect(lua_State* lua)
	{
		const std::string addr = luaL_checkstring(lua, 1);
		ClientComponent * clientComponent = App::Get<ClientComponent>();
		{
			int sessionId = clientComponent->Connect(addr);
			lua_pushinteger(lua, sessionId);
		}
		return 1;
	}

	int LuaClient::Close(lua_State* l)
	{
		int sessionId = (int)luaL_checkinteger(l, 1);
		ClientComponent * clientComponent = App::Get<ClientComponent>();
		{
			clientComponent->Remove(sessionId);
		}
		return 0;
	}
}