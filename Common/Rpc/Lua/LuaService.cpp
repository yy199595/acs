//
// Created by yjz on 2022/5/23.
//

#include"LuaService.h"
#include"Entity/Unit/App.h"
#include"Util/Json/Lua/Json.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Util/String/StringHelper.h"
#include"Rpc/Async/RpcTaskSource.h"
#include"Proto/Component/ProtoComponent.h"
#include"Rpc/Component/NodeMgrComponent.h"
#include"Rpc/Component/InnerNetComponent.h"
#include"Rpc/Component/DispatchComponent.h"
using namespace Tendo;
namespace Lua
{
	int Service::Call(lua_State* lua)
    {
        InnerNetComponent * pNetComponent = App::Inst()->GetComponent<InnerNetComponent>();
        DispatchComponent * pMessageComponent = App::Inst()->GetComponent<DispatchComponent>();
		if (pNetComponent == nullptr || pMessageComponent == nullptr)
		{
			luaL_error(lua, "InnerNetComponent Is Null");
			return 0;
		}

		std::shared_ptr<Msg::Packet> request(new Msg::Packet());
        {
            request->SetType(Msg::Type::Request);
            request->SetProto(Msg::Porto::Protobuf);
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
            std::string server;
            NodeMgrComponent* locationComponent = 
                App::Inst()->GetComponent<NodeMgrComponent>();
            if (!ClusterConfig::Inst()->GetServerName(methodConfig->Service, server))
            {
                luaL_error(lua, "not cluster config %s", methodConfig->Service.c_str());
                return 0;
            }
            locationComponent->GetServer(server, userId, address);
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
		int rpdId = 0;
        lua_pushthread(lua);
        const std::string &response = methodConfig->Response;
		request->GetHead().Add("func", methodConfig->FullName);
        if (!pNetComponent->Send(address, request, rpdId))
        {
            luaL_error(lua, "send request message error");
            return 0;
        }
		std::shared_ptr<LuaRpcTaskSource> luaRpcTaskSource
			= std::make_shared<LuaRpcTaskSource>(lua, rpdId, response);
        return pMessageComponent->AddTask(rpdId, luaRpcTaskSource)->Await();
    }

	int Service::AllotServer(lua_State *lua)
	{
        std::string server;
		const std::string service = luaL_checkstring(lua, 1);
        if (!ClusterConfig::Inst()->GetServerName(service, server))
        {
            luaL_error(lua, "not find server : service = %s ", service.c_str());
            return 0;
        }
		NodeMgrComponent * locationComponent = App::Inst()->GetComponent<NodeMgrComponent>();
		if(lua_isinteger(lua, 2))
		{
			std::string address;
			long long userId = lua_tointeger(lua, 2);
            if (locationComponent->GetServer(server, userId, address))
            {
                lua_pushlstring(lua, address.c_str(), address.size());
                return 1;
            }
		}
		else if(lua_isstring(lua, 2))
		{
			std::string address;
			const char * listen = lua_tostring(lua, 2);
			if(locationComponent->GetServer(server, address, listen))
			{
				lua_pushlstring(lua, address.c_str(), address.size());
				return 1;
			}
		}
		else
		{
			std::string address;
			const char * listen = "rpc";
			if(locationComponent->GetServer(server, address, listen))
			{
				lua_pushlstring(lua, address.c_str(), address.size());
				return 1;
			}
		}
		return 0;
	}

    int Service::Send(lua_State* lua)
    {
        std::shared_ptr<Msg::Packet> request(new Msg::Packet());
        {
            request->SetType(Msg::Type::Request);
            request->SetProto(Msg::Porto::Protobuf);
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
            
            NodeMgrComponent* locationComponent = 
                App::Inst()->GetComponent<NodeMgrComponent>();
            if (locationComponent->GetServer(methodConfig->Server, userId, address))
            {
                return 0;
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
        //const std::string& response = methodConfig->Response;
        request->GetHead().Add("func", methodConfig->FullName);
        InnerNetComponent * pNetComponent = App::Inst()->GetComponent<InnerNetComponent>();
        if (pNetComponent == nullptr)
        {
            luaL_error(lua, "DispatchMessageComponent Is Null");
            return 0;
        }
        if (!pNetComponent->Send(address, request))
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
		const char * listen = "rpc";
		const char * service = luaL_checkstring(lua, 1);
		if(lua_isstring(lua, 2))
		{
			listen = lua_tostring(lua, 2);
		}
        if (!ClusterConfig::Inst()->GetServerName(service, server))
        {
            return 0;
        }
		std::vector<std::string> servers;
		NodeMgrComponent* locationComponent = App::Inst()->GetComponent<NodeMgrComponent>();
        if (locationComponent != nullptr && locationComponent->GetServer(server, servers, listen))
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