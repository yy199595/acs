//
// Created by mac on 2022/5/30.
//

#include"LuaSocket.h"
#include"Entity/Actor/App.h"
#include"Util/Tools/String.h"
#include"Udp/Common/UdpClient.h"
#include"Net/Network/Tcp/Socket.h"
#include"Lua/Engine/UserDataParameter.h"
#include"Udp/Component/UdpComponent.h"
#include"Kcp/Common/KcpClient.h"
#include"Kcp/Component/KcpComponent.h"
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
				lua_pushinteger(L, count);
				return 1;
			}
			catch (const asio::system_error& code)
			{
				luaL_error(L, "send : %s", code.what());
				return 0;
			}
		}
		return 0;
	}

	int TcpSock::Read(lua_State* L)
	{
		tcp::Socket* sock = Lua::UserDataParameter::Read<tcp::Socket*>(L, 1);
		{
			size_t size = 0;
			std::string result;
			std::vector<char> buffer(1024);
			Asio::Socket& socket = sock->Get();
			try
			{
				do
				{
					size = socket.read_some(asio::buffer(buffer));
					if (size <= 0)
					{
						break;
					}
					result.append(buffer.data(), size);
					std::this_thread::sleep_for(std::chrono::microseconds(1));
				}
				while (size > 0 && socket.available() > 0);
			}
			catch (const std::system_error& code)
			{
				luaL_error(L, "read : %s", code.what());
				return 0;
			}			
			lua_pushlstring(L, result.c_str(), result.size());
			return 1;
		}
	}

	int TcpSock::Close(lua_State* L)
	{
		tcp::Socket* sock = Lua::UserDataParameter::Read<tcp::Socket*>(L, 1);
		{
			sock->Close();
			delete sock;
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
		Asio::Context& io = acs::App::GetContext();
		tcp::Socket* socket = new tcp::Socket(io);
		try
		{
			if (help::Str::IsIpAddress(host))
			{
				Asio::Address address = asio::ip::make_address(host);
				{
					Asio::EndPoint endPoint(address, port);
					socket->Get().connect(endPoint);
				}
				Lua::UserDataParameter::Write(L, socket);
				return 1;
			}
			Asio::Resolver resolver(io);
			Asio::ResolverQuery query(host, std::to_string(port));
			asio::connect(socket->Get(), resolver.resolve(query));

			Lua::UserDataParameter::Write(L, socket);
			return 1;
		}
		catch (const asio::system_error& code)
		{
			delete socket;
			luaL_error(L, "connect : %s", code.what());
			return 0;
		}
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
		int id = (int)luaL_checkinteger(L, 2);
		Asio::Context & ctx = acs::App::GetContext();
		acs::UdpComponent * udpComponent = acs::App::Get<acs::UdpComponent>();
		asio::ip::udp::endpoint remote(asio::ip::make_address(ip), port);
		udp::Client * udpClient = new udp::Client(ctx, udpComponent, remote);
		udpClient->StartReceive();
		Lua::UserDataParameter::Write(L, udpClient);
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

	int UdpSock::Close(lua_State* L)
	{
		udp::Client * udpClient = Lua::UserDataParameter::Read<udp::Client*>(L, 1);
		if(udpClient == nullptr)
		{
			return 0;
		}
		delete udpClient;
		lua_pushboolean(L, true);
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
		Asio::Context & context = acs::App::GetContext();
		asio::ip::udp::endpoint endpoint(asio::ip::make_address(ip), port);
		acs::KcpComponent * kcpComponent = acs::App::Get<acs::KcpComponent>();
		kcp::Client * kcpClient = new kcp::Client(context, kcpComponent, endpoint);
		{
			Lua::UserDataParameter::Write(L, kcpClient);
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
