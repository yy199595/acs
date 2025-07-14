//
// Created by leyi on 2023/11/8.
//
#include<string>
#include"LuaClient.h"
#include"XCode/XCode.h"
#include"Rpc/Common/Message.h"
#include"Lua/Engine/Define.h"
#include"Entity/Actor/App.h"
#include"Router/Component/RouterComponent.h"
#include "Client/Component/WsClientComponent.h"
#include "Client/Component/TcpClientComponent.h"

namespace lua
{
	int Client::Send(lua_State* l)
	{
		std::unique_ptr<rpc::Message> message;
		int sessionId = (int)luaL_checkinteger(l, 1);
		const std::string func = luaL_checkstring(l, 2);
		if(acs::App::Inst()->MakeMessage(l, 3, func, message) != XCode::Ok)
		{
			lua_pushinteger(l, XCode::Failure);
			return 1;
		}
		acs::RouterComponent * router = acs::App::Get<acs::RouterComponent>();
		if(router == nullptr)
		{
			lua_pushinteger(l, XCode::NetWorkError);
			return 1;
		}
		message->SetNet(rpc::net::client);
		lua_pushinteger(l, router->Send(sessionId, message));
		return 1;
	}

	int Client::Call(lua_State* l)
	{
		std::unique_ptr<rpc::Message> message;
		int sessionId = (int)luaL_checkinteger(l, 1);
		const std::string func = luaL_checkstring(l, 2);
		if(acs::App::Inst()->MakeMessage(l, 3, func, message) != XCode::Ok)
		{
			lua_pushinteger(l, XCode::Failure);
			return 1;
		}
		acs::RouterComponent * router = acs::App::Get<acs::RouterComponent>();
		if(router == nullptr)
		{
			lua_pushinteger(l, XCode::NetWorkError);
			return 1;
		}

		lua_pushthread(l);
		message->SetNet(rpc::net::client);
		return router->LuaCall(l, sessionId, message);
	}

	int Client::Connect(lua_State* lua)
	{
		const std::string addr = luaL_checkstring(lua, 1);
		acs::RouterComponent * router = acs::App::Get<acs::RouterComponent>();
		if(router == nullptr)
		{
			lua_pushinteger(lua, XCode::NetWorkError);
			return 1;
		}
		rpc::IInnerSender* clientSender = router->GetSender(rpc::net::client);
		if(clientSender == nullptr)
		{
			luaL_error(lua, "not find client sender");
			return 0;
		}
		int sessionId = clientSender->Connect(addr);
		lua_pushinteger(lua, sessionId);
		return 1;
	}

	int Client::Close(lua_State* L)
	{
		int sessionId = (int)luaL_checkinteger(L, 1);
		acs::RouterComponent * router = acs::App::Get<acs::RouterComponent>();
		if(router == nullptr)
		{
			lua_pushinteger(L, XCode::NetWorkError);
			return 1;
		}
		rpc::IInnerSender* clientSender = router->GetSender(rpc::net::client);
		if(clientSender == nullptr)
		{
			luaL_error(L, "not find client sender");
			return 0;
		}
		clientSender->Remove(sessionId);
		return 0;
	}
}


namespace lua
{
	int lib::luaopen_lclient(lua_State* L)
	{
		luaL_Reg l[] = {
				{ "Send", lua::Client::Send },
				{ "Call", lua::Client::Call },
				{ "Close", lua::Client::Close },
				{ "Connect", lua::Client::Connect },
				{ nullptr,  nullptr }
		};
		luaL_newlib(L, l);
		return 1;
	}
}