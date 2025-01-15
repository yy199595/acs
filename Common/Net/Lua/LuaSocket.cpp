//
// Created by mac on 2022/5/30.
//

#include"LuaSocket.h"
#include"Entity/Actor/App.h"
#include"Util/Tools/String.h"
#include"Udp/Common/UdpClient.h"
#include"Network/Tcp/Socket.h"
#include"Lua/Engine/UserDataParameter.h"
#include"Udp/Component/UdpComponent.h"
#include"Kcp/Common/KcpClient.h"
#include"Kcp/Component/KcpComponent.h"
#include "Async/Lua/LuaWaitTaskSource.h"

namespace Lua
{
	int TcpSock::Send(lua_State* L)
	{
		tcp::Socket* sock = Lua::UserDataParameter::Read<tcp::Socket*>(L, 1);
		{
			size_t size = 0;
			const char* buff = luaL_checklstring(L, 2, &size);
			try
			{
				size_t count = sock->Get().write_some(asio::buffer(buff, size));
				lua_pushinteger(L, (int)count);
				return 1;
			}
			catch (const asio::system_error& code)
			{
				luaL_error(L, "send : %s", code.what());
				return 0;
			}
		}
	}

	int TcpSock::Read(lua_State* L)
	{
		tcp::Socket* sock = Lua::UserDataParameter::Read<tcp::Socket*>(L, 1);
		{
			size_t size = 0;
			std::string result;
			char buffer[128] = { 0 };
			do
			{
				Asio::Code code;
				Asio::Socket& socket = sock->Get();
				size = socket.read_some(asio::buffer(buffer), code);
				if (size > 0)
				{
					result.append(buffer, size);
				}
				if(code.value() != Asio::OK)
				{
					if(code != asio::error::eof)
					{
						luaL_error(L, "%s", code.message().c_str());
						return 0;
					}
				}
			} while (size > 0);

			lua_pushlstring(L, result.c_str(), result.size());
			return 1;
		}
	}

	int TcpSock::Query(lua_State* L)
	{
		std::vector<std::string> result;
		std::string host(luaL_checkstring(L, 1));
		if (help::Str::IsIpAddress(host))
		{
			result.emplace_back(host);
		}
		else
		{
			Asio::Context& io = acs::App::Inst()->GetContext();

			Asio::Resolver resolver(io);
			Asio::ResolverQuery query(host, "");
			auto iter = resolver.resolve(query);
			Asio::Resolver::iterator end;
			for (; iter != end; iter++)
			{
				result.emplace_back(iter->endpoint().address().to_string());
			}
		}
		lua_newtable(L);
		int top = lua_gettop(L);
		for (size_t index = 0; index < result.size(); index++)
		{
			lua_pushinteger(L, index + 1);
			const std::string& str = result.at(index);
			lua_pushlstring(L, str.c_str(), str.size());
			lua_settable(L, -3);
		}
		lua_settop(L, top);
		return 1;
	}

	int TcpSock::Connect(lua_State* L)
	{
		std::string host(luaL_checkstring(L, 1));
		const int port = (int)luaL_checkinteger(L, 2);
		Asio::Context& io = acs::App::Inst()->GetContext();
		std::unique_ptr<tcp::Socket> socket = std::make_unique<tcp::Socket>(io);
		try
		{
			if (help::Str::IsIpAddress(host))
			{
				Asio::Address address = asio::ip::make_address(host);
				{
					Asio::EndPoint endPoint(address, port);
					socket->Get().connect(endPoint);
				}
				Lua::UserDataParameter::UserDataStruct<tcp::Socket*>::WritePtr(L, socket.release());
				return 1;
			}
			Asio::Resolver resolver(io);
			Asio::ResolverQuery query(host, std::to_string(port));
			asio::connect(socket->Get(), resolver.resolve(query));
			Lua::UserDataParameter::UserDataStruct<tcp::Socket*>::WritePtr(L, socket.release());
			return 1;
		}
		catch (const asio::system_error& code)
		{
			luaL_error(L, "connect : %s", code.what());
			return 0;
		}
	}
}
