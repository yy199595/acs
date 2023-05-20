//
// Created by yjz on 2022/5/15.
//

#include"LuaApp.h"
#include"Entity/Actor/App.h"
#include"XCode/XCode.h"
#include"Util/Json/Lua/Json.h"
#include"Entity/Unit/Player.h"
#include"Server/Config/ServiceConfig.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Rpc/Async/RpcTaskSource.h"
#include"Lua/Engine/LocalClassProxy.h"
#include"Proto/Component/ProtoComponent.h"
#include"Rpc/Component/InnerNetComponent.h"
#include"Rpc/Component/DispatchComponent.h"
#include"Entity/Component/PlayerMgrComponent.h"
using namespace Tendo;
namespace Lua
{
	int LuaApp::GetComponent(lua_State* lua)
	{
		const char* name = luaL_checkstring(lua, -1);
		Component* component = App::Inst()->GetComponentByName(name);
        if(component == nullptr)
        {
            return 0;
        }
		typedef UserDataParameter::UserDataStruct<Component*> ComponentType;
		return ComponentType::WriteObj(lua, component, name);
	}

	int LuaApp::MakeRequest(lua_State * lua, std::shared_ptr<Msg::Packet> & message, std::string & addr)
	{
		const std::string func = luaL_checkstring(lua, 2);
		const RpcMethodConfig* methodConfig = RpcConfig::Inst()->GetMethodConfig(func);
		if (methodConfig == nullptr)
		{
			luaL_error(lua, "not find rpc config : %s", func.c_str());
			return XCode::NotFoundRpcConfig;
		}
		message = std::make_shared<Msg::Packet>();
		{
			message->SetType(Msg::Type::Request);
			message->GetHead().Add("func", func);
		}
		if(lua_isinteger(lua, 1))
		{
			long long userId = lua_tointeger(lua, 1);
			PlayerMgrComponent * playerMgrComponent = App::Inst()->GetComponent<PlayerMgrComponent>();
			if(playerMgrComponent == nullptr)
			{
				luaL_error(lua, "not find PlayerMgrComponent");
				return XCode::Failure;
			}
			Player * player = playerMgrComponent->GetPlayer(userId);
			if(player == nullptr)
			{
				luaL_error(lua, "not find user : %lld", userId);
				return XCode::NotFindUser;
			}
			if(!player->GetAddress(methodConfig->Server, addr))
			{
				return XCode::NotFoundPlayerRpcAddress;
			}
			message->GetHead().Add("id", userId);
		}
		else if(lua_isstring(lua, 1))
		{
			addr = lua_tostring(lua, 1);
		}
		else
		{
			return XCode::CallArgsError;
		}

		if (lua_istable(lua, 3))
		{
			if (!methodConfig->Request.empty())
			{
				std::shared_ptr<pb::Message> request;
				const std::string& name = methodConfig->Request;
				if (App::Inst()->GetProto()->Read(lua, name, 3, request))
				{
					luaL_error(lua, "create request message error : %s", name.c_str());
					return XCode::CreateProtoFailure;
				}
				message->SetProto(Msg::Porto::Protobuf);
				message->WriteMessage(request.get());
			}
			else
			{
				message->SetProto(Msg::Porto::Json);
				Lua::RapidJson::Read(lua, 3, message->Body());
			}
		}
		else if(lua_isstring(lua, 3))
		{
			size_t size = 0;
			message->SetProto(Msg::Porto::String);
			const char * str = luaL_tolstring(lua, 3, &size);
			message->Body()->append(str, size);
		}
		message->SetFrom(methodConfig->Response);
		return XCode::Successful;
	}

	int LuaApp::GetAddr(lua_State * lua)
	{
		std::string server, addr;
		const std::string service = luaL_checkstring(lua, 1);
		if(!ClusterConfig::Inst()->GetServerName(service, server))
		{
			luaL_error(lua, "not find service name : %s", service.c_str());
			return 0;
		}
		// TODO
		lua_pushlstring(lua, addr.c_str(), addr.size());
		return 1;
	}

	int LuaApp::GetActor(lua_State * lua)
	{
		App * app = App::Inst();
		if(lua_isnil(lua, -1))
		{
			Lua::Parameter::Write(lua, app);
			return 1;
		}
		long long actorId = luaL_checkinteger(lua, 1);
		Actor * actor = app->GetActorMgr()->GetActor(actorId);
		Lua::Parameter::Write(lua, actor);
		return 1;
	}

	int LuaApp::Send(lua_State* lua)
	{
		std::string addr;
		App * app = App::Inst();
		std::shared_ptr<Msg::Packet> message;
		int code = LuaApp::MakeRequest(lua, message, addr);
		if(code != XCode::Successful)
		{
			lua_pushinteger(lua, code);
			return 1;
		}
		app->GetComponent<InnerNetComponent>()->Send(addr, message);
		return 1;
	}

	int LuaApp::Call(lua_State* lua)
	{
		std::string addr;
		lua_pushthread(lua);
		App * app = App::Inst();
		std::shared_ptr<Msg::Packet> message;
		int code = LuaApp::MakeRequest(lua, message, addr);
		if(code != XCode::Successful)
		{
			lua_pushinteger(lua, code);
			return 1;
		}
		return app->GetComponent<InnerNetComponent>()->LuaCall(lua, addr, message);
	}
}