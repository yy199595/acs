//
// Created by yjz on 2022/5/23.
//

#include"LuaService.h"
#include"Entity/Unit/App.h"
#include"Util/Json/Lua/Json.h"
#include"Async/Lua/LuaCoroutine.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Rpc/Async/RpcTaskSource.h"
#include"Proto/Component/ProtoComponent.h"
#include"Rpc/Component/LocationComponent.h"
#include"Rpc/Component/InnerNetComponent.h"
#include"Rpc/Component/DispatchComponent.h"
using namespace Tendo;
namespace Lua
{
	int Service::Call(lua_State* lua)
    {
		assert(Lua::Coroutine::IsRunning(lua));
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
		int targetId = (int)luaL_checkinteger(lua, 1);
		if(lua_isinteger(lua, 2))
		{
			long long useId = lua_tointeger(lua, 2);
			if(useId > 0)
			{
				request->GetHead().Add("id", useId);
			}
		}
		std::string fullName = luaL_checkstring(lua, 3);
		const RpcMethodConfig* methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
		if (methodConfig == nullptr)
		{
			luaL_error(lua, "call service parameter error");
			return 0;
		}
		if(lua_istable(lua, 4))
		{
			if (!methodConfig->Request.empty())
			{
				const std::string& pb = methodConfig->Request;
				ProtoComponent* messageComponent = App::Inst()->GetProto();
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
				request->SetProto(Msg::Porto::Protobuf);
			}
			else
			{
				request->SetProto(Msg::Porto::Json);
				Lua::RapidJson::Read(lua, 4, request->Body());
			}
		}
		else if(lua_isstring(lua, 4))
		{
			size_t size = 0;
			const char * str = luaL_tolstring(lua, 4, &size);
			request->Body()->append(str, size);
			request->SetProto(Msg::Porto::String);
		}

		int rpdId = 0;
        lua_pushthread(lua);
        const std::string &response = methodConfig->Response;
		request->GetHead().Add("func", methodConfig->FullName);
        if (!pNetComponent->Send(targetId, request, rpdId))
        {
            luaL_error(lua, "send request message error");
            return 0;
        }
		std::shared_ptr<LuaRpcTaskSource> luaRpcTaskSource
			= std::make_shared<LuaRpcTaskSource>(lua, rpdId, response);
        return pMessageComponent->AddTask(rpdId, luaRpcTaskSource)->Await();
    }

	int Service::GetTarget(lua_State* lua)
	{
		std::string server;
		long long userId = luaL_checkinteger(lua, 1);
		const char * service = luaL_checkstring(lua, 2);
		if (!ClusterConfig::Inst()->GetServerName(service, server))
		{
			luaL_error(lua, "not find server : service = %s ", service);
			return 0;
		}
		LocationComponent * locationComponent = App::Inst()->GetComponent<LocationComponent>();
		if(locationComponent == nullptr)
		{
			luaL_error(lua, "not find LocationComponent");
			return 0;
		}
		ClientUnit * clientUnit = locationComponent->GetClientById(userId);
		if(clientUnit == nullptr)
		{
			return 0;
		}
		int serverId = 0;
		if(clientUnit->Get(server, serverId))
		{
			lua_pushinteger(lua, serverId);
			return 1;
		}
		return 0;
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
		LocationComponent * locationComponent = App::Inst()->GetComponent<LocationComponent>();
		if(locationComponent == nullptr)
		{
			luaL_error(lua, "not find LocationComponent");
			return 0;
		}
		int targetId = locationComponent->RangeServer(server);
		lua_pushinteger(lua, targetId);
		return 1;
	}

    int Service::Send(lua_State* lua)
    {
        std::shared_ptr<Msg::Packet> request(new Msg::Packet());
        {
			request->SetProto(Msg::Porto::None);
			request->SetType(Msg::Type::Request);
        }
		int targetId = luaL_checkinteger(lua, 1);
		if(lua_isinteger(lua, 2))
		{
			long long useId = lua_tointeger(lua, 2);
			if(useId > 0)
			{
				request->GetHead().Add("id", useId);
			}
		}
		std::string fullName = luaL_checkstring(lua, 3);
		const RpcMethodConfig* methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
        if (methodConfig == nullptr)
        {
            luaL_error(lua, "call service parameter error");
            return 0;
        }
		if(lua_istable(lua, 4))
		{
			if (!methodConfig->Request.empty())
			{
				const std::string& pb = methodConfig->Request;
				ProtoComponent* messageComponent = App::Inst()->GetProto();
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
				request->SetProto(Msg::Porto::Protobuf);
			}
			else
			{
				request->SetProto(Msg::Porto::Json);
				Lua::RapidJson::Read(lua, 4, request->Body());
			}
		}
		else if(lua_isstring(lua, 4))
		{
			size_t size = 0;
			const char * str = luaL_tolstring(lua, 4, &size);
			request->Body()->append(str, size);
			request->SetProto(Msg::Porto::String);
		}

        lua_pushthread(lua);
        request->GetHead().Add("func", methodConfig->FullName);
        InnerNetComponent * pNetComponent = App::Inst()->GetComponent<InnerNetComponent>();
        if (pNetComponent == nullptr)
        {
            luaL_error(lua, "DispatchMessageComponent Is Null");
            return 0;
        }
        if (!pNetComponent->Send(targetId, request))
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

	int Service::RangeServer(lua_State* lua)
	{
		std::string server;
		const char * service = luaL_checkstring(lua, 1);
		if(!ClusterConfig::Inst()->GetServerName(service, server))
		{
			luaL_error(lua, "not find server : %s", service);
			return 0;
		}
		LocationComponent * locationComponent = App::Inst()->GetComponent<LocationComponent>();
		if(locationComponent == nullptr)
		{
			luaL_error(lua, "not find LocationComponent");
			return 0;
		}
		int id = locationComponent->RangeServer(server);
		if(id == -1)
		{
			return 0;
		}
		lua_pushinteger(lua, id);
		return 1;
	}

	int Service::GetAddrById(lua_State* lua)
	{
		int id = luaL_checkinteger(lua, 1);
		const char * listen = luaL_checkstring(lua, 2);
		LocationComponent * locationComponent = App::Inst()->GetComponent<LocationComponent>();
		if(locationComponent == nullptr)
		{
			luaL_error(lua, "not find LocationComponent");
			return 0;
		}
		std::string address;
		if(!locationComponent->GetServerAddress(id, listen, address))
		{
			return 0;
		}
		lua_pushlstring(lua, address.c_str(), address.size());
		return 1;
	}

    int Service::AddLocation(lua_State *lua)
    {
        return 0;
    }

    int Service::GetServerList(lua_State* lua)
    {
        return 0;
    }
}