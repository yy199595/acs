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
		if(lua_isstring(l, 1))
		{
			const char* json = luaL_checklstring(l, 1, &size);
			if (!document.Decode(json, size))
			{
				luaL_error(l, "decode json fail");
				return 0;
			}
		}
		else if(lua_istable(l, 1))
		{
			std::string json;
			lua::yyjson::read(l, 1, json);
			if (!document.Decode(json))
			{
				luaL_error(l, "decode json fail");
				return 0;
			}
		}
		int id = 0;
		std::string name;
		if(!document.Get("id", id) || !document.Get("name", name))
		{
			luaL_error(l, "not id or name");
			return 0;
		}
		Actor * target = (Server *)App::ActorMgr()->GetActor(id);
		if(target == nullptr)
		{
			std::unique_ptr<Server> server = std::make_unique<Server>(id, name);
			{
				target = server.get();
				App::ActorMgr()->AddActor(std::move(server));
			}
		}
		if(!target->Decode(document))
		{
			App::ActorMgr()->DelActor(id);
			return 0;
		}
		App::ActorMgr()->AddGroup(target->Name(), id);
		lua_pushboolean(l, true);
		return 1;
	}

	int LuaActor::RemoveServer(lua_State* L)
	{
		long long id = luaL_checkinteger(L, 1);
		lua_pushboolean(L, App::ActorMgr()->DelActor(id));
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
		std::unique_ptr<rpc::Message> message;
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
		std::unique_ptr<rpc::Message> message;
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
		std::unique_ptr<rpc::Message> message;
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

		std::string name(luaL_checkstring(lua, 1));
		actor::Group * actorGroup = actComponent->GetGroup(name);
		if (actorGroup == nullptr)
		{
			std::string serverName;
			if (!ClusterConfig::Inst()->GetServerName(name, serverName))
			{
				return 0;
			}
			actorGroup = actComponent->GetGroup(serverName);
			if(actorGroup == nullptr)
			{
				return 0;
			}
		}
		const std::vector<long long> & items = actorGroup->GetItems();
		lua_createtable(lua, 0, (int)items.size());
		for (int index = 0; index < items.size(); index++)
		{
			lua_pushinteger(lua, items[index]);
			lua_seti(lua, -2, index + 1);
		}
		return 1;
	}

	int LuaActor::AddListen(lua_State* lua)
	{
		Server* server = App::Inst();
		if (lua_isnumber(lua, 1))
		{
			int id = (int)luaL_checkinteger(lua, 1);
			server = dynamic_cast<Server*>(App::ActorMgr()->GetActor(id));
			if (server == nullptr)
			{
				luaL_error(lua, "not find server:%d", id);
				return 0;
			}
		}
		std::string name(luaL_checkstring(lua, 2));
		std::string address(luaL_checkstring(lua, 3));
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
			server = dynamic_cast<Server*>(App::ActorMgr()->GetActor(id));
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

	int LuaActor::AllInfo(lua_State* L)
	{
		Server* server = App::Inst();
		if(lua_isinteger(L, 1))
		{
			long long id = luaL_checkinteger(L, 1);
			server = dynamic_cast<Server*>(App::ActorMgr()->GetActor(id));
		}
		if(server == nullptr)
		{
			return 0;
		}
		json::w::Document document;
		server->Encode(document);
		std::string json = document.JsonString();
		lua::yyjson::write(L, json.c_str(), json.size());
		return 1;
	}
}