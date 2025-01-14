//
// Created by mac on 2022/5/30.
//

#include"LuaSocket.h"
#include"Entity/Actor/App.h"
#include"Util/Tools/String.h"
#include"Udp/Common/UdpClient.h"
#include "Lua/Socket/LuaClient.h"
#include"Lua/Engine/UserDataParameter.h"
#include"Udp/Component/UdpComponent.h"
#include"Kcp/Common/KcpClient.h"
#include"Kcp/Component/KcpComponent.h"
#include "Server/Component/ThreadComponent.h"
namespace Lua
{
	int TcpSock::Send(lua_State* L)
	{
		std::shared_ptr<lua::Client> luaClient = Lua::UserDataParameter::Read<std::shared_ptr<lua::Client>>(L, 1);
		{
			lua_pushthread(L);
			auto taskSource = std::make_unique<acs::LuaWaitTaskSource>(L);

			size_t size = 0;
			const char* buff = luaL_checklstring(L, 2, &size);
			return luaClient->Send(std::string(buff, size), std::move(taskSource));
		}
	}

	int TcpSock::Read(lua_State* L)
	{
		std::shared_ptr<lua::Client> luaClient = Lua::UserDataParameter::Read<std::shared_ptr<lua::Client>>(L, 1);
		{
			lua_pushthread(L);
			auto taskSource = std::make_unique<acs::LuaWaitTaskSource>(L);

			return luaClient->Read(std::move(taskSource));
		}
	}

	int TcpSock::Close(lua_State* L)
	{
		std::shared_ptr<lua::Client> luaClient = Lua::UserDataParameter::Read<std::shared_ptr<lua::Client>>(L, 1);
		{
			luaClient->Close();
		}
		return 0;
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
		Asio::Context& main = acs::App::Inst()->GetContext();
		Asio::Context& context = acs::App::Get<acs::ThreadComponent>()->GetContext();
		std::unique_ptr<tcp::Socket> socket = std::make_unique<tcp::Socket>(context);
		{
			socket->Init(host, port);
		}
		std::shared_ptr<lua::Client> luaClient = std::make_shared<lua::Client>(main, socket.release());
		if (!luaClient->Start())
		{
			luaL_error(L, "connect [%s:%d]", host.c_str(), port);
			return 0;
		}
		Lua::UserDataParameter::UserDataStruct<std::shared_ptr<lua::Client>>::Write(L, luaClient);
		return 1;
	}

	int UdpSock::New(lua_State* L)
	{
		std::string ip;
		unsigned short port = 0;
		std::string addr(luaL_checkstring(L, 1));
		if(!help::Str::SplitAddr(addr, ip, port))
		{
			luaL_error(L, "decode %s fail", addr.c_str());
			return 0;
		}
		Asio::Context& ctx = acs::App::Inst()->GetContext();
		asio::ip::udp::endpoint remote(asio::ip::make_address(ip), port);
		acs::UdpComponent * udpComponent = acs::App::Get<acs::UdpComponent>();
		udp::Client * udpClient = new udp::Client(ctx, udpComponent, remote, ctx);
		{
			udpClient->StartReceive();
			Lua::UserDataParameter::UserDataStruct<udp::Client*>::WritePtr(L, udpClient);
		}
		return 1;
	}

	int UdpSock::Send(lua_State* L)
	{
		udp::Client * udpClient = Lua::UserDataParameter::Read<udp::Client*>(L, 1);
		if(udpClient == nullptr)
		{
			return 0;
		}
		size_t size = 0;
		const char * message = luaL_checklstring(L, 2, &size);
		tcp::TextProto * textProto = new tcp::TextProto(message, size);
		{
			udpClient->Send(textProto);
			lua_pushboolean(L, true);
		}
		return 1;
	}

	int KcpSock::New(lua_State* L)
	{
		std::string ip;
		unsigned short port = 0;
		std::string address(luaL_checkstring(L, 1));
		if(!help::Str::SplitAddr(address, ip, port))
		{
			return 0;
		}
		Asio::Context& context = acs::App::Inst()->GetContext();
		asio::ip::udp::endpoint endpoint(asio::ip::make_address(ip), port);
		acs::KcpComponent * kcpComponent = acs::App::Get<acs::KcpComponent>();
		kcp::Client * kcpClient = new kcp::Client(context, kcpComponent, endpoint, context);
		{
			Lua::UserDataParameter::UserDataStruct<kcp::Client*>::WritePtr(L, kcpClient);
		}
		return 1;
	}

	int KcpSock::Send(lua_State* L)
	{
		kcp::Client * kcpClient = Lua::UserDataParameter::Read<kcp::Client*>(L, 1);
		if(kcpClient == nullptr)
		{
			lua_error(L);
			return 0;
		}
		size_t size = 0;
		const char * message = luaL_checklstring(L, 2, &size);
		kcpClient->Send(new tcp::TextProto(message, size));
		lua_pushboolean(L, true);
		return 1;
	}
}
