//
// Created by leyi on 2023/5/25.
//

#include "LuaActor.h"
#include "XCode/XCode.h"
#include "Entity/Actor/App.h"
#include "Lua/Engine/Define.h"
#include "Util/Json/Lua/Json.h"
#include "Rpc/Config/ClientConfig.h"
#include "Cluster/Config/ClusterConfig.h"
#include "Proto/Component/ProtoComponent.h"

namespace Tendo
{
	int LuaActor::NewServer(lua_State* lua)
	{
		std::string name;
		int id = (int)luaL_checkinteger(lua, 1);
		std::string addr(luaL_checkstring(lua, 2));
		if(lua_isstring(lua, 3))
		{
			name = lua_tostring(lua, 3);
		}
		ActorComponent * actorComponent = App::Inst()->ActorMgr();
		std::shared_ptr<ServerActor> actor = std::make_shared<ServerActor>(id, name);
		{
			actor->AddListen("rpc", addr);
			actorComponent->AddServer(actor);
		}
		return 0;
	}

	int LuaActor::LuaPushCode(lua_State * l, int code)
	{
		lua_pushinteger(l, code);
		return 1;
	}

	int LuaActor::Send(lua_State* lua)
	{
		Actor * targetAcotr = nullptr;
		if(lua_isnil(lua, 1))
		{
			targetAcotr = App::Inst();
		}
		else
		{
			long long actorId = luaL_checkinteger(lua, 1);
			targetAcotr = App::Inst()->ActorMgr()->GetActor(actorId);
			if(targetAcotr == nullptr)
			{
				LOG_ERROR("not find actor : " << actorId);
				return LuaActor::LuaPushCode(lua, XCode::NotFoundActor);
			}
		}
		std::shared_ptr<Msg::Packet> message;
		const std::string func(luaL_checkstring(lua, 2));
		const int code = targetAcotr->MakeMessage(lua, 3, func, message);
		if(code != XCode::Successful)
		{
			return LuaActor::LuaPushCode(lua, code);
		}
		return targetAcotr->LuaSend(lua, func, message);
	}

	int LuaActor::Call(lua_State* lua)
	{
		Actor * targetActor = nullptr;
		const std::string func = luaL_checkstring(lua, 2);
		if(lua_isnil(lua, 1))
		{
			targetActor = App::Inst();
		}
		else
		{
			long long actorId = luaL_checkinteger(lua, 1);
			targetActor = App::Inst()->ActorMgr()->GetActor(actorId);
			if(targetActor == nullptr)
			{
				LOG_ERROR("call [" << func << "] fail not find actor:" << actorId);
				return LuaActor::LuaPushCode(lua, XCode::NotFoundActor);
			}
		}

		if(targetActor == nullptr)
		{
			return LuaActor::LuaPushCode(lua, XCode::NotFindUser);
		}
		std::shared_ptr<Msg::Packet> message;
		const int code = targetActor->MakeMessage(lua, 3, func, message);
		if(code != XCode::Successful)
		{
			return LuaActor::LuaPushCode(lua, code);
		}
		lua_pushthread(lua);
		return targetActor->LuaCall(lua, func, message);
	}

	int LuaActor::AddWatch(lua_State* lua)
	{
		std::string service(luaL_checkstring(lua, 1));
		lua_pushboolean(lua, App::Inst()->AddWatch(service));
		return 1;
	}

	int LuaActor::SendToClient(lua_State* lua)
	{
		std::string response;
		long long playerId = luaL_checkinteger(lua, 1);
		const std::string func(luaL_checkstring(lua, 2));
		if (!ClientConfig::Inst()->GetConfig(func, response))
		{
			LOG_ERROR("not config client : " << func);
			return LuaActor::LuaPushCode(lua, XCode::NotFoundRpcConfig);
		}
		std::shared_ptr<Msg::Packet> message = std::make_shared<Msg::Packet>();
		{
			message->SetType(Msg::Type::Client);
			message->GetHead().Add("func", func);
		}
		PlayerActor* player = App::Inst()->ActorMgr()->GetPlayer(playerId);
		if (player == nullptr)
		{
			LOG_FMT_ERR("send message func:{} user:{}", func, playerId);
			return LuaActor::LuaPushCode(lua, XCode::NotFindUser);
		}
		if (lua_istable(lua, 3))
		{
			if (response == "json")
			{
				message->SetProto(Msg::Porto::Json);
				Lua::RapidJson::Read(lua, 3, message->Body());
			}
			else if (!response.empty())
			{
				std::shared_ptr<pb::Message> request;
				if (!App::Inst()->GetProto()->Read(lua, response, 3, request))
				{
					LOG_ERROR("create pb fail : " << response);
					return LuaActor::LuaPushCode(lua, XCode::CreateProtoFailure);
				}

				message->GetHead().Add("pb", response);
				message->SetProto(Msg::Porto::Protobuf);
				if (!message->WriteMessage(request.get()))
				{
					LOG_ERROR("writer message fail : " << response);
					return LuaActor::LuaPushCode(lua, XCode::SerializationFailure);
				}
			}
		}
		else if (lua_isstring(lua, 3))
		{
			size_t size = 0;
			const char* data = luaL_tolstring(lua, 3, &size);
			message->Body()->append(data, size);
			message->SetProto(Msg::Porto::String);
		}
		int code = player->SendToClient(message);
		return LuaActor::LuaPushCode(lua, code);
	}

	int LuaActor::Random(lua_State* lua)
	{
		if(lua_isnil(lua, 1))
		{
			lua_pushinteger(lua, App::Inst()->GetActorId());
			return 1;
		}
		else if(lua_isstring(lua, 1))
		{
			std::string server;
			std::string name(lua_tostring(lua, 1));
			if (!ClusterConfig::Inst()->GetServerName(name, server))
			{
				LOG_ERROR("not find server by service :" << name);
				return 0;
			}
			ServerActor* actor = App::Inst()->ActorMgr()->Random(server);
			if (actor == nullptr)
			{
				LOG_ERROR("allot server fail : " << server);
				return 0;
			}
			lua_pushinteger(lua, actor->GetActorId());
			return 1;
		}
		return 0;
	}

	int LuaActor::GetListen(lua_State* lua)
	{
		std::string address;
		long long id = luaL_checkinteger(lua, 1);
		const std::string name(luaL_checkstring(lua, 2));
		ServerActor * server = App::Inst()->ActorMgr()->GetServer(id);
		if(server == nullptr || !server->GetListen(name, address))
		{
			return 0;
		}
		lua_pushlstring(lua, address.c_str(), address.size());
		return 1;
	}
}