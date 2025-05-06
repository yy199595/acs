#include "LuaActor.h"
#include "XCode/XCode.h"
#include "Entity/Actor/App.h"
#include "Lua/Engine/Define.h"
#include "Yyjson/Lua/ljson.h"
#include "Lua/Component/LuaComponent.h"
#include "Cluster/Config/ClusterConfig.h"
#include "Proto/Component/ProtoComponent.h"

namespace acs
{
	int LuaApp::GetPath(lua_State* l)
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

	int LuaApp::NewGuid(lua_State* l)
	{
		long long guid = App::Inst()->MakeGuid();
		lua_pushinteger(l, guid);
		return 1;
	}

	int LuaApp::NewUuid(lua_State* l)
	{
		std::string guid = App::Inst()->NewUuid();
		lua_pushlstring(l, guid.c_str(), guid.size());
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
		const std::string component(luaL_checkstring(lua, 1));
		IActorComponent * actorComponent = App::Inst()->GetComponent<IActorComponent>(component);
		if(actorComponent == nullptr)
		{
			luaL_error(lua, "not find ActorComponent");
			return 0;
		}
		Actor* targetActor = nullptr;
		if (lua_isnil(lua, 2))
		{
			targetActor = App::Inst();
		}
		else
		{
			int nodeId = (int)luaL_checkinteger(lua, 2);
			targetActor = actorComponent->GetActor(nodeId);
			if (targetActor == nullptr)
			{
				LOG_ERROR("not find actor:{}", nodeId);
				return LuaActor::LuaPushCode(lua, XCode::NotFoundActor);
			}
		}
		std::unique_ptr<rpc::Message> message;
		const std::string func(luaL_checkstring(lua, 3));
		int code = targetActor->MakeMessage(lua, 4, func, message);
		if (code != XCode::Ok)
		{
			return LuaActor::LuaPushCode(lua, code);
		}
		return targetActor->LuaSend(lua, std::move(message));
	}

	int LuaActor::Call(lua_State* lua)
	{
        lua_pushthread(lua);
		std::string component(luaL_checkstring(lua, 1));
		IActorComponent * actorComponent = App::Inst()->GetComponent<IActorComponent>(component);
		if(actorComponent == nullptr)
		{
			luaL_error(lua, "not find ActorComponent");
			return 0;
		}

		Actor* targetActor = nullptr;
		const std::string func = luaL_checkstring(lua, 3);
		if (lua_isnil(lua, 2))
		{
			targetActor = App::Inst();
		}
		else
		{
			long long actorId = luaL_checkinteger(lua, 2);
			targetActor = actorComponent->GetActor(actorId);
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
		std::unique_ptr<rpc::Message> message;
		int code = targetActor->MakeMessage(lua, 4, func, message);
		if (code != XCode::Ok)
		{
			return LuaActor::LuaPushCode(lua, code);
		}
		return targetActor->LuaCall(lua, std::move(message));
	}

	int LuaApp::GetConfig(lua_State* lua)
	{
		const std::string json = ServerConfig::Inst()->ToString();
		if (!lua::yyjson::write(lua, json.c_str(), json.size()))
		{
			return 0;
		}
		return 1;
	}

	int LuaApp::HasComponent(lua_State* l)
	{
		std::string name(luaL_checkstring(l, 1));
		bool result = App::Inst()->HasComponent(name);
		lua_pushboolean(l, result);
		return 1;
	}

	int LuaActor::GetServers(lua_State* lua)
	{
		NodeComponent* actComponent = App::ActorMgr();
		if (actComponent == nullptr)
		{
			luaL_error(lua, "not find ActorComponent");
			return 0;
		}

		std::string name(luaL_checkstring(lua, 1));
		NodeCluster * nodeCluster = actComponent->GetCluster(name);
		if (nodeCluster == nullptr)
		{
			std::string serverName;
			if (!ClusterConfig::Inst()->GetServerName(name, serverName))
			{
				return 0;
			}
			nodeCluster = actComponent->GetCluster(serverName);
			if(nodeCluster == nullptr)
			{
				return 0;
			}
		}
		const std::vector<int> & items = nodeCluster->GetNodes();
		lua_createtable(lua, 0, (int)items.size());
		for (int index = 0; index < items.size(); index++)
		{
			lua_pushinteger(lua, items[index]);
			lua_seti(lua, -2, index + 1);
		}
		return 1;
	}

	int LuaActor::Broadcast(lua_State* L)
	{
		const std::string component(luaL_checkstring(L, 1));
		IActorComponent * actorComponent = App::Inst()->GetComponent<IActorComponent>(component);
		if(actorComponent == nullptr)
		{
			luaL_error(L, "not find ActorComponent");
			return 0;
		}
		std::unique_ptr<rpc::Message> message;
		const std::string func(luaL_checkstring(L, 2));
		int code = acs::App::Inst()->MakeMessage(L, 3, func, message);
		if (code != XCode::Ok)
		{
			return LuaActor::LuaPushCode(L, code);
		}
		int count = 0;
		int result = 0;
		code = actorComponent->Broadcast(std::move(message), count);
		{
			lua_pushinteger(L, code);
			lua_pushinteger(L, count);
		}
		return 2;
	}


}
