//
// Created by yjz on 2022/5/15.
//

#include"LuaApp.h"
#include"App/App.h"
#include"Service/RpcService.h"
#include"Async/RpcTaskSource.h"
#include"Config/ServiceConfig.h"
#include"Component/ProtoComponent.h"
#include"Component/InnerNetMessageComponent.h"
using namespace Sentry;
namespace Lua
{
	int LuaApp::Send(lua_State* lua)
	{
		const char * fullName = luaL_checkstring(lua, 1);
		const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
		InnerNetMessageComponent *netMessageComponent = App::Inst()->GetComponent<InnerNetMessageComponent>();

		std::shared_ptr<Rpc::Packet> message = std::make_shared<Rpc::Packet>();
		{
			message->SetType(Tcp::Type::Request);
			message->SetProto(Tcp::Porto::Protobuf);
			message->GetHead().Add("func", methodConfig->FullName);
		}
		int index = 2;
		if(lua_isinteger(lua, 2))
		{
			index++;
			long long userId = lua_tointeger(lua, 2);
			message->GetHead().Add("id", userId);
		}
		if(!methodConfig->Request.empty())
		{
			const std::string &pb = methodConfig->Request;
			ProtoComponent *messageComponent = App::Inst()->GetMsgComponent();
			std::shared_ptr<Message> request = messageComponent->Read(lua, pb, index);
			if (request == nullptr)
			{
				luaL_error(lua, "lua call %s request is empty", fullName);
				return 0;
			}
			if(request->GetTypeName() != methodConfig->Request)
			{
				luaL_error(lua, "lua call %s request type error", fullName);
				return 0;
			}
			message->WriteMessage(request.get());
		}
		std::string address;
		ServerConfig::Inst()->GetLocation("rpc", address);
		lua_pushboolean(lua, netMessageComponent->Send(address, message));
		return 1;
	}
	int LuaApp::Call(lua_State* lua)
	{
		const char * fullName = luaL_checkstring(lua, 1);
		const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
		InnerNetMessageComponent *netMessageComponent = App::Inst()->GetComponent<InnerNetMessageComponent>();

		std::shared_ptr<Rpc::Packet> message = std::make_shared<Rpc::Packet>();
		{
			message->SetType(Tcp::Type::Request);
			message->SetProto(Tcp::Porto::Protobuf);
			message->GetHead().Add("func", methodConfig->FullName);
		}
		int index = 2;
		if(lua_isinteger(lua, 2))
		{
			index++;
			long long userId = lua_tointeger(lua, 2);
			message->GetHead().Add("id", userId);
		}
		if(!methodConfig->Request.empty())
		{
			const std::string &pb = methodConfig->Request;
			ProtoComponent *messageComponent = App::Inst()->GetMsgComponent();
			std::shared_ptr<Message> request = messageComponent->Read(lua, pb, index);
			if (request == nullptr)
			{
				luaL_error(lua, "lua call %s request is empty", fullName);
				return 0;
			}
			if(request->GetTypeName() != methodConfig->Request)
			{
				luaL_error(lua, "lua call %s request type error", fullName);
				return 0;
			}
			message->WriteMessage(request.get());
		}
		lua_pushthread(lua);
		const std::string &response = methodConfig->Response;
		std::shared_ptr<LuaRpcTaskSource> luaRpcTaskSource
			= std::make_shared<LuaRpcTaskSource>(lua, 0, response);
		{
			message->GetHead().Add("rpc", luaRpcTaskSource->GetRpcId());
		}
		std::string address;
		ServerConfig::Inst()->GetLocation("rpc", address);
		if (!netMessageComponent->Send(address, message))
		{
			luaL_error(lua, "send request message error");
			return 0;
		}
		return netMessageComponent->AddTask(luaRpcTaskSource)->Await();
	}

	int LuaApp::GetComponent(lua_State* lua)
	{
		const char* name = luaL_checkstring(lua, -1);
		Component* component = App::Inst()->GetComponentByName(name);
        if(component == nullptr)
        {
            lua_pushnil(lua);
            return 1;
        }
        const std::string fullName = fmt::format("App{0}", name);
		return UserDataParameter::UserDataStruct<Component*>::WriteObj(lua, component, fullName.c_str());
	}
}