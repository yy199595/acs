//
// Created by leyi on 2023/5/25.
//

#include "LuaActor.h"
#include "XCode/XCode.h"
#include "Entity/Actor/App.h"
#include "Lua/Engine/Define.h"
#include "Util/Json/Lua/Json.h"
#include "Proto/Component/ProtoComponent.h"
namespace Tendo
{
	int LuaActor::Send(lua_State* lua)
	{
		const long long actorId = luaL_checkinteger(lua, 1);
		const std::string func(luaL_checkstring(lua, 2));
		Actor * actor = App::Inst()->ActorMgr()->GetActor(actorId);
		if(actor == nullptr)
		{
			lua_pushinteger(lua, XCode::NotFindUser);
			return 1;
		}
		std::shared_ptr<Msg::Packet> message;
		const int code = actor->MakeMessage(lua, 3, func, message);
		if(code != XCode::Successful)
		{
			lua_pushinteger(lua, code);
			return 1;
		}
		return actor->LuaSend(lua, func, message);
	}

	int LuaActor::Call(lua_State* lua)
	{
		const long long actorId = luaL_checkinteger(lua, 1);
		const std::string func = luaL_checkstring(lua, 2);
		Actor * actor = App::Inst()->ActorMgr()->GetActor(actorId);
		if(actor == nullptr)
		{
			lua_pushinteger(lua, XCode::NotFindUser);
			return 1;
		}
		std::shared_ptr<Msg::Packet> message;
		const int code = actor->MakeMessage(lua, 3, func, message);
		if(code != XCode::Successful)
		{
			lua_pushinteger(lua, code);
			return 1;
		}
		return actor->LuaCall(lua, func, message);
	}

	int LuaActor::SendToClient(lua_State* lua)
	{
		int code = XCode::Successful;
		long long playerId = luaL_checkinteger(lua, 1);
		const std::string func(luaL_checkstring(lua, 2));
		std::shared_ptr<Msg::Packet> message = std::make_shared<Msg::Packet>();
		{
			message->SetType(Msg::Type::Client);
			message->GetHead().Add("func", func);
		}
		PlayerActor * player = nullptr;
		do
		{
			player = App::Inst()->ActorMgr()->GetPlayer(playerId);
			if(player == nullptr)
			{
				code = XCode::NotFindUser;
				break;
			}
			if(lua_isstring(lua, 3))
			{
				const std::string data(lua_tostring(lua, 3));
				if(lua_istable(lua, 4))
				{
					if (data == "json")
					{
						message->SetProto(Msg::Porto::Json);
						Lua::RapidJson::Read(lua, 4, message->Body());
						break;
					}
					std::shared_ptr<pb::Message> request;
					if (!App::Inst()->GetProto()->Read(lua, data, 4, request))
					{
						code = XCode::CreateProtoFailure;
						break;
					}

					message->GetHead().Add("pb", data);
					message->SetProto(Msg::Porto::Protobuf);
					if (!message->WriteMessage(request.get()))
					{
						code = XCode::SerializationFailure;
						break;
					}
					break;
				}
				message->Body()->append(data);
				message->SetProto(Msg::Porto::String);
			}
		} while (false);
		if(code != XCode::Successful)
		{
			lua_pushinteger(lua, code);
			return 1;
		}
		lua_pushinteger(lua, player->SendToClient(message));
		return 1;
	}

	int LuaActor::Random(lua_State* lua)
	{
		if(lua_isstring(lua, 1))
		{

			std::string name(lua_tostring(lua, 1));
			ServerActor * actor = App::Inst()->ActorMgr()->Random(name);
			if(actor == nullptr)
			{
				return 0;
			}
			lua_pushinteger(lua, actor->GetActorId());
			return 1;
		}
		lua_pushinteger(lua, App::Inst()->GetActorId());
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