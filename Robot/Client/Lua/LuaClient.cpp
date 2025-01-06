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

namespace acs
{
	int LuaTcpClient::Send(lua_State* l)
	{
		std::unique_ptr<rpc::Message> message;
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

	int LuaTcpClient::Call(lua_State* l)
	{
		std::unique_ptr<rpc::Message> message;
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
		lua_pushthread(l);
		message->SetNet(rpc::Net::Client);
		return router->LuaCall(l, sessionId, std::move(message));
	}

	int LuaTcpClient::Connect(lua_State* lua)
	{
		const std::string addr = luaL_checkstring(lua, 1);
		TcpClientComponent * clientComponent = App::Get<TcpClientComponent>();
		{
			int sessionId = clientComponent->Connect(addr);
			lua_pushinteger(lua, sessionId);
		}
		return 1;
	}

	int LuaTcpClient::Close(lua_State* l)
	{
		int sessionId = (int)luaL_checkinteger(l, 1);
		TcpClientComponent * clientComponent = App::Get<TcpClientComponent>();
		{
			clientComponent->Remove(sessionId);
		}
		return 0;
	}
}

namespace acs
{
	int LuaWsClient::Send(lua_State* l)
	{
		std::unique_ptr<rpc::Message> message;
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

	int LuaWsClient::Call(lua_State* l)
	{
		std::unique_ptr<rpc::Message> message;
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
		lua_pushthread(l);
		message->SetNet(rpc::Net::Client);
		return router->LuaCall(l, sessionId, std::move(message));
	}

	int LuaWsClient::Connect(lua_State* lua)
	{
		const std::string addr = luaL_checkstring(lua, 1);
		WsClientComponent * clientComponent = App::Get<WsClientComponent>();
		{
			int sessionId = clientComponent->Connect(addr);
			lua_pushinteger(lua, sessionId);
		}
		return 1;
	}

	int LuaWsClient::Close(lua_State* l)
	{
		int sessionId = (int)luaL_checkinteger(l, 1);
		WsClientComponent * clientComponent = App::Get<WsClientComponent>();
		{
			clientComponent->Remove(sessionId);
		}
		return 0;
	}
}
