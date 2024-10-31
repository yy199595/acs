#include "LuaActor.h"
#include "XCode/XCode.h"
#include "Entity/Actor/App.h"
#include "Lua/Engine/Define.h"
#include "Yyjson/Lua/ljson.h"
#include "Lua/Component/LuaComponent.h"
#include "Cluster/Config/ClusterConfig.h"
#include "Proto/Component/ProtoComponent.h"
#include"Cluster/Component/LaunchComponent.h"

namespace acs
{
	int LuaActor::GetPath(lua_State* l)
	{
		std::string path;
		const std::string key(luaL_checkstring(l, 1));
		if (!App::Inst()->Config().GetPath(key, path))
		{
			return 0;
		}
		lua_pushstring(l, path.c_str());
		return 1;
	}

	int LuaActor::NewGuid(lua_State* l)
	{
		long long guid = App::Inst()->MakeGuid();
		lua_pushinteger(l, guid);
		return 1;
	}

	int LuaActor::NewUuid(lua_State* l)
	{
		std::string guid = App::Inst()->NewUuid();
		lua_pushlstring(l, guid.c_str(), guid.size());
		return 1;
	}

	int LuaActor::MakeServer(lua_State* l)
	{
		size_t size = 0;
		json::r::Document document;
		const char * json = luaL_checklstring(l, 1, &size);
		if(!document.Decode(json, size))
		{
			luaL_error(l, "decode json fail");
			return 0;
		}
		ActorComponent * actMgr = App::ActorMgr();
		Server * server = actMgr->MakeServer(document);
		if(server == nullptr)
		{
			luaL_error(l, "create server fail");
			return 0;
		}
		bool res = actMgr->AddServer(server);
		lua_pushboolean(l, res);
		return 1;
	}

	int LuaActor::AddComponent(lua_State* l)
	{
		std::string name = luaL_checkstring(l, 1);
		Component* component = App::Inst()->GetComponentByName(name);
		if (component != nullptr)
		{
			lua_pushboolean(l, true);
			return 1;
		}
		if (!App::Inst()->AddComponent(name))
		{
			LaunchComponent* launchComponent = App::Get<LaunchComponent>();
			if (launchComponent == nullptr)
			{
				luaL_error(l, "not find LaunchComponent");
				return 0;
			}
			if (!launchComponent->AddService(name))
			{
				luaL_error(l, "add %s fail", name.c_str());
				return 0;
			}
		}
		component = App::Inst()->GetComponentByName(name);
		if (component == nullptr || !component->LateAwake())
		{
			return 0;
		}

		lua_pushboolean(l, true);
		return 1;
	}

	int LuaActor::LuaPushCode(lua_State* l, int code)
	{
		lua_pushinteger(l, code);
		return 1;
	}

	int LuaActor::Stop(lua_State* l)
	{
		CoroutineComponent * coroutine = App::Get<CoroutineComponent>();
		coroutine->Start(&App::Stop, App::Inst());
		return 0;
	}

	int LuaActor::Send(lua_State* lua)
	{
		ActorComponent* actComponent = App::ActorMgr();
		if (actComponent == nullptr)
		{
			luaL_error(lua, "not find ActorComponent");
			return 0;
		}

		Actor* targetActor = nullptr;
		if (lua_isnil(lua, 1))
		{
			targetActor = App::Inst();
		}
		else
		{
			long long actorId = luaL_checkinteger(lua, 1);
			targetActor = actComponent->GetActor(actorId);
			if (targetActor == nullptr)
			{
				LOG_ERROR("not find actor:{}", actorId);
				return LuaActor::LuaPushCode(lua, XCode::NotFoundActor);
			}
		}
		std::unique_ptr<rpc::Packet> message;
		const std::string func(luaL_checkstring(lua, 2));
		int code = targetActor->MakeMessage(lua, 3, func, message);
		if (code != XCode::Ok)
		{
			return LuaActor::LuaPushCode(lua, code);
		}
		return targetActor->LuaSend(lua, std::move(message));
	}

	int LuaActor::Call(lua_State* lua)
	{
        lua_pushthread(lua);
		ActorComponent* actComponent = App::ActorMgr();
		if (actComponent == nullptr)
		{
			luaL_error(lua, "not find ActorComponent");
			return 0;
		}

		Actor* targetActor = nullptr;
		const std::string func = luaL_checkstring(lua, 2);
		if (lua_isnil(lua, 1))
		{
			targetActor = App::Inst();
		}
		else
		{
			long long actorId = luaL_checkinteger(lua, 1);
			targetActor = actComponent->GetActor(actorId);
			if (targetActor == nullptr)
			{
				LOG_ERROR("call {} fail not find actor:{}", func, actorId);
				return LuaActor::LuaPushCode(lua, XCode::NotFoundActor);
			}
		}

		if (targetActor == nullptr)
		{
			return LuaActor::LuaPushCode(lua, XCode::NotFindUser);
		}
		std::unique_ptr<rpc::Packet> message;
		int code = targetActor->MakeMessage(lua, 3, func, message);
		if (code != XCode::Ok)
		{
			return LuaActor::LuaPushCode(lua, code);
		}
		return targetActor->LuaCall(lua, std::move(message));
	}

	int LuaActor::Publish(lua_State* lua)
	{
		ActorComponent* actComponent = App::ActorMgr();
		if (actComponent == nullptr)
		{
			luaL_error(lua, "not find ActorComponent");
			return 0;
		}

		Actor* targetActor = App::Inst();
		if(lua_isnumber(lua, 1))
		{
			long long actorId = luaL_checkinteger(lua, 1);
			targetActor = actComponent->GetActor(actorId);
			if (targetActor == nullptr)
			{
				LOG_ERROR("not find actor:{}", actorId);
				return LuaActor::LuaPushCode(lua, XCode::NotFoundActor);
			}
		}
		std::unique_ptr<rpc::Packet> message;
		std::string channel(luaL_checkstring(lua, 2));
		const std::string func("EventSystem.Publish");
		int code = targetActor->MakeMessage(lua, 3, func, message);
		if (code != XCode::Ok)
		{
			return LuaActor::LuaPushCode(lua, code);
		}
		message->GetHead().Add("channel", channel);
		return targetActor->LuaSend(lua, std::move(message));
	}

	int LuaActor::GetConfig(lua_State* lua)
	{
		const std::string json = ServerConfig::Inst()->ToString();
		if (!lua::yyjson::write(lua, json.c_str(), json.size()))
		{
			return 0;
		}
		return 1;
	}

	int LuaActor::HasComponent(lua_State* l)
	{
		std::string name(luaL_checkstring(l, 1));
		bool result = App::Inst()->HasComponent(name);
		lua_pushboolean(l, result);
		return 1;
	}

	int LuaActor::GetServers(lua_State* lua)
	{
		ActorComponent* actComponent = App::ActorMgr();
		if (actComponent == nullptr)
		{
			luaL_error(lua, "not find ActorComponent");
			return 0;
		}

		std::vector<int> servers;
		std::string name(luaL_checkstring(lua, 1));
		actComponent->GetServers(name, servers);
		if (servers.empty())
		{
			std::string serverName;
			if (ClusterConfig::Inst()->GetServerName(name, serverName))
			{
				actComponent->GetServers(serverName, servers);
			}
		}
		lua_createtable(lua, 0, (int)servers.size());
		for (int index = 0; index < servers.size(); index++)
		{
			lua_pushinteger(lua, servers[index]);
			lua_seti(lua, -2, index + 1);
		}
		return 1;
	}

	int LuaActor::Random(lua_State* lua)
	{
		if (lua_isnil(lua, 1))
		{
			lua_pushinteger(lua, App::Inst()->GetSrvId());
			return 1;
		}

		std::string server;
		std::string name(luaL_checkstring(lua, 1));
		if (!ClusterConfig::Inst()->GetServerName(name, server))
		{
			LOG_ERROR("not find server by service : {}", name);
			return 0;
		}
		Server* targetServer = nullptr;
		ActorComponent* actorComponent = App::ActorMgr();

		if (lua_isinteger(lua, 2))
		{
			long long id = lua_tointeger(lua, 2);
			targetServer = actorComponent->Hash(name, id);
		}
		else
		{
			targetServer = actorComponent->Random(server);
			if (targetServer == nullptr && App::Inst()->HasComponent(name))
			{
				targetServer = App::Inst();
			}
		}
		if (targetServer == nullptr)
		{
			luaL_error(lua, "allocator %s fail", name.c_str());
			return 0;
		}
		lua_pushinteger(lua, targetServer->GetSrvId());
		return 1;
	}

	int LuaActor::AddListen(lua_State* lua)
	{
		Server* server = App::Inst();
		if (lua_isnumber(lua, 1))
		{
			long long id = luaL_checkinteger(lua, 1);
			server = App::ActorMgr()->GetServer(id);
		}
		std::string name(luaL_checkstring(lua, 2));
		std::string address(luaL_checkstring(lua, 3));
		if (server == nullptr)
		{
			return 0;
		}
		lua_pushboolean(lua, server->AddListen(name, address));
		return 1;
	}

	int LuaActor::GetListen(lua_State* lua)
	{
		std::string address;
		Server* server = App::Inst();
		if(lua_isinteger(lua, 1))
		{
			long long id = luaL_checkinteger(lua, 1);
			server = App::ActorMgr()->GetServer(id);
		}
		if (server == nullptr)
		{
			return 0;
		}
		std::string name(luaL_checkstring(lua, 2));
		if (!server->GetListen(name, address))
		{
			return 0;
		}
		lua_pushlstring(lua, address.c_str(), address.size());
		return 1;
	}

	int LuaPlayer::AddAddr(lua_State* lua)
	{
		long long playerId = luaL_checkinteger(lua, 1);
		Player* player = App::ActorMgr()->GetPlayer(playerId);
		if (player == nullptr)
		{
			luaL_error(lua, "not find player:{}", playerId);
			return 0;
		}
		std::string sever = luaL_checkstring(lua, 2);
		long long serverId = luaL_checkinteger(lua, 3);
		player->AddAddr(sever, (int)serverId);
		return LuaActor::LuaPushCode(lua, XCode::Ok);
	}
}