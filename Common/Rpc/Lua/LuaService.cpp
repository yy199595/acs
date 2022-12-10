//
// Created by yjz on 2022/5/23.
//

#include"LuaService.h"
#include"App/App.h"
#include"Json/Lua/Json.h"
#include"Config/ClusterConfig.h"
#include"String/StringHelper.h"
#include"Async/RpcTaskSource.h"
#include"Component/ProtoComponent.h"
#include"Component/LocationComponent.h"
#include"Component/InnerNetMessageComponent.h"
using namespace Sentry;
namespace Lua
{
	int Service::Call(lua_State* lua)
    {
		std::shared_ptr<Rpc::Packet> request(new Rpc::Packet());
        {
            request->SetType(Tcp::Type::Request);
            request->SetProto(Tcp::Porto::Protobuf);
        }
		std::string address;
		const RpcMethodConfig * methodConfig = nullptr;
        if (lua_isnil(lua, 1))
        {
            ServerConfig::Inst()->GetLocation("rpc", address);
            const std::string fullName = luaL_checkstring(lua, 2);
            methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
        }
		else if (lua_isinteger(lua, 1)) //userId
        {
            long long userId = lua_tointeger(lua, 1);
            request->GetHead().Add("id", userId);
#ifdef __INNER_MSG_FORWARD__
            ForwardHelperComponent * forwardComponent = App::Inst()->GetComponent<ForwardHelperComponent>();
            forwardComponent->GetLocation(userId, address);
#else
			std::string fullName = luaL_checkstring(lua, 2);
			methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
			if(methodConfig == nullptr)
			{
				luaL_error(lua, "not find config %s", fullName.c_str());
				return 0;
			}
			LocationComponent * locationComponent = App::Inst()->GetComponent<LocationComponent>();
            const LocationUnit * locationUnit = locationComponent->GetUnit(userId);
            if(locationUnit == nullptr || (!locationUnit->Get(methodConfig->Service, address)))
            {
				locationComponent->GetTranLocation(userId, address);
            }
#endif
        }
        else if (lua_isstring(lua, 1)) //address
        {
#ifdef __INNER_MSG_FORWARD__
            const char * to = lua_tostring(lua, 2);
            request->GetHead().Add("to", std::string(to));
            ForwardHelperComponent * forwardComponent = App::Inst()->GetComponent<ForwardHelperComponent>();
            forwardComponent->GetLocation(address);
#else
            address.append(lua_tostring(lua, 1));
#endif
			std::string fullName = luaL_checkstring(lua, 2);
			methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
			if(methodConfig == nullptr)
			{
				luaL_error(lua, "not find config %s", fullName.c_str());
				return 0;
			}
        }
        if (address.empty() || methodConfig == nullptr)
        {
            luaL_error(lua, "call service parameter error");
            return 0;
        }
        if (!methodConfig->Request.empty())
        {
			int t1 = lua_type(lua, 3);
            const std::string &pb = methodConfig->Request;
            ProtoComponent *messageComponent = App::Inst()->GetMsgComponent();
            std::shared_ptr<Message> message = messageComponent->Read(lua, pb, 3);
            if (message == nullptr)
            {
                luaL_error(lua, "lua call %s request is empty", methodConfig->FullName.c_str());
                return 0;
            }
            if(message->GetTypeName() != methodConfig->Request)
            {
                luaL_error(lua, "lua call %s request type error", methodConfig->FullName.c_str());
                return 0;
            }
            request->WriteMessage(message.get());
        }

        lua_pushthread(lua);
        const std::string &response = methodConfig->Response;
        std::shared_ptr<LuaRpcTaskSource> luaRpcTaskSource
            = std::make_shared<LuaRpcTaskSource>(lua, 0, response);
		{
			request->GetHead().Add("func", methodConfig->FullName);
			request->GetHead().Add("rpc", luaRpcTaskSource->GetRpcId());
		}
		InnerNetMessageComponent *netMessageComponent = App::Inst()->GetComponent<InnerNetMessageComponent>();
		if (netMessageComponent == nullptr)
		{
			luaL_error(lua, "InnerNetMessageComponent Is Null");
			return 0;
		}
        if (!netMessageComponent->Send(address, request))
        {
            luaL_error(lua, "send request message error");
            return 0;
        }
        long long id = luaRpcTaskSource->GetRpcId();
        return netMessageComponent->AddTask(id, luaRpcTaskSource)->Await();
    }

	int Service::AllotServer(lua_State *lua)
	{
		const std::string service = luaL_checkstring(lua, 1);
		LocationComponent * locationComponent = App::Inst()->GetComponent<LocationComponent>();
		if(lua_isinteger(lua, 2))
		{
			std::string address;
			long long userId = lua_tointeger(lua, 2);
			LocationUnit * locationUnit = locationComponent->GetUnit(userId);
			if(locationUnit != nullptr && locationUnit->Get(service, address))
			{
				lua_pushlstring(lua, address.c_str(), address.size());
				return 1;
			}
		}
        std::string server;
        if (!ClusterConfig::Inst()->GetServerName(service, server))
        {
            luaL_error(lua, "not find server : service = %s ", service.c_str());
            return 0;
        }
		std::string address;
		if(locationComponent->AllotServer(server, address))
		{
			lua_pushlstring(lua, address.c_str(), address.size());
			return 1;
		}
		return 0;
	}

    int Service::Send(lua_State* lua)
    {
        std::shared_ptr<Rpc::Packet> request(new Rpc::Packet());
        {
            request->SetType(Tcp::Type::Request);
            request->SetProto(Tcp::Porto::Protobuf);
        }
        std::string address;
        const RpcMethodConfig* methodConfig = nullptr;
        if (lua_isnil(lua, 1))
        {
            ServerConfig::Inst()->GetLocation("rpc", address);
            const std::string fullName = luaL_checkstring(lua, 2);
            methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
        }
        else if (lua_isinteger(lua, 1)) //userId
        {
            long long userId = lua_tointeger(lua, 1);
            request->GetHead().Add("id", userId);
#ifdef __INNER_MSG_FORWARD__
            ForwardHelperComponent* forwardComponent = App::Inst()->GetComponent<ForwardHelperComponent>();
            forwardComponent->GetLocation(userId, address);
#else
            std::string fullName = luaL_checkstring(lua, 2);
            methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
            if (methodConfig == nullptr)
            {
                luaL_error(lua, "not find config %s", fullName.c_str());
                return 0;
            }
            LocationComponent* locationComponent = App::Inst()->GetComponent<LocationComponent>();
            const LocationUnit* locationUnit = locationComponent->GetUnit(userId);
            if (locationUnit == nullptr || (!locationUnit->Get(methodConfig->Service, address)))
            {
				locationComponent->GetTranLocation(userId, address);
            }
#endif
        }
        else if (lua_isstring(lua, 1)) //address
        {
#ifdef __INNER_MSG_FORWARD__
            const char* to = lua_tostring(lua, 2);
            request->GetHead().Add("to", std::string(to));
            ForwardHelperComponent* forwardComponent = App::Inst()->GetComponent<ForwardHelperComponent>();
            forwardComponent->GetLocation(address);
#else
            address.append(lua_tostring(lua, 1));
#endif
            std::string fullName = luaL_checkstring(lua, 2);
            methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
            if (methodConfig == nullptr)
            {
                luaL_error(lua, "not find config %s", fullName.c_str());
                return 0;
            }
        }
        if (address.empty() || methodConfig == nullptr)
        {
            luaL_error(lua, "call service parameter error");
            return 0;
        }
        if (!methodConfig->Request.empty())
        {
            int t1 = lua_type(lua, 3);
            const std::string& pb = methodConfig->Request;
            ProtoComponent* messageComponent = App::Inst()->GetMsgComponent();
            std::shared_ptr<Message> message = messageComponent->Read(lua, pb, 3);
            if (message == nullptr)
            {
                luaL_error(lua, "lua call %s request is empty", methodConfig->FullName.c_str());
                return 0;
            }
            if (message->GetTypeName() != methodConfig->Request)
            {
                luaL_error(lua, "lua call %s request type error", methodConfig->FullName.c_str());
                return 0;
            }
            request->WriteMessage(message.get());
        }

        lua_pushthread(lua);
        const std::string& response = methodConfig->Response;
        request->GetHead().Add("func", methodConfig->FullName);
        InnerNetMessageComponent* netMessageComponent = App::Inst()->GetComponent<InnerNetMessageComponent>();
        if (netMessageComponent == nullptr)
        {
            luaL_error(lua, "InnerNetMessageComponent Is Null");
            return 0;
        }
        if (!netMessageComponent->Send(address, request))
        {
            luaL_error(lua, "send request message error");
            return 0;
        }
        lua_pushboolean(lua, true);
        return 1;
    }

    int Service::FindService(lua_State *lua)
    {
        std::string service = luaL_checkstring(lua, 1);
        RpcService * rpcService = App::Inst()->GetService(service);
        lua_pushboolean(lua, rpcService != nullptr);
        return 1;
    }

    int Service::AddLocation(lua_State *lua)
    {
        return 0;
    }

    int Service::GetServerList(lua_State* lua)
    {
        std::string server;
        std::vector<std::string> servers;
        const std::string service = luaL_checkstring(lua, 1);
        if (!ClusterConfig::Inst()->GetServerName(service, server))
        {
            return 0;
        }
        LocationComponent* locationComponent = App::Inst()->GetComponent<LocationComponent>();
        if (locationComponent != nullptr && locationComponent->GetServers(server, servers))
        {
            lua_newtable(lua);
            int top = lua_gettop(lua);
          
            for (size_t index = 0; index < servers.size(); index++)
            {
                lua_pushinteger(lua, index);
                const std::string& data = servers[index];
                lua_pushlstring(lua, data.c_str(), data.size());
                lua_settable(lua, -3);
            }
            lua_settop(lua, top);
            return 1;
        }
        return 0;
    }
}