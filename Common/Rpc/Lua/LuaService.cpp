//
// Created by yjz on 2022/5/23.
//

#include"LuaService.h"
#include"App/App.h"
#include"Json/Lua/Json.h"
#include"String/StringHelper.h"
#include"Async/RpcTaskSource.h"
#include"Component/ProtoComponent.h"
#include"Component/LocationComponent.h"
#include"Component/InnerNetMessageComponent.h"
#include"Component/ForwardHelperComponent.h"
using namespace Sentry;
namespace Lua
{
	int Service::Call(lua_State* lua)
    {
        if (!lua_isuserdata(lua, 1))
        {
            luaL_error(lua, "call paremeter first is service");
            return 0;
        }
        std::string address;
        Sentry::RpcService *service = UserDataParameter::Read<Sentry::RpcService *>(lua, 1);
        InnerNetMessageComponent *netMessageComponent = App::Inst()->GetComponent<InnerNetMessageComponent>();
        if (netMessageComponent == nullptr)
        {
            luaL_error(lua, "not find InnerNetMessageComponent");
            return 0;
        }
        std::shared_ptr<Rpc::Packet> request(new Rpc::Packet());
        {
            request->SetType(Tcp::Type::Request);
            request->SetProto(Tcp::Porto::Protobuf);
        }
        if (lua_isinteger(lua, 2)) //userId
        {
            long long userId = lua_tointeger(lua, 2);
            request->GetHead().Add("id", userId);
#ifdef __INNER_MSG_FORWARD__
            ForwardHelperComponent * forwardComponent = App::Inst()->GetComponent<ForwardHelperComponent>();
            forwardComponent->GetLocation(userId, address);
#else
            LocationComponent * locationComponent = App::Inst()->GetComponent<LocationComponent>();
            const LocationUnit * locationUnit = locationComponent->GetLocationUnit(userId);
            if(locationUnit == nullptr || (!locationUnit->Get(service->GetName(), address)))
            {
                ForwardHelperComponent * forwardComponent = App::Inst()->GetComponent<ForwardHelperComponent>();
                forwardComponent->GetLocation(userId, address);
            }
#endif
        }
        else if (lua_isstring(lua, 2)) //address
        {
#ifdef __INNER_MSG_FORWARD__
            const char * to = lua_tostring(lua, 2);
            request->GetHead().Add("to", std::string(to));
            ForwardHelperComponent * forwardComponent = App::Inst()->GetComponent<ForwardHelperComponent>();
            forwardComponent->GetLocation(address);
#else
            address.append(lua_tostring(lua, 2));
#endif
        }
        if (address.empty())
        {
            luaL_error(lua, "call service address is empty");
            return 0;
        }
        if (!lua_isstring(lua, 3))
        {
            luaL_error(lua, "not find service function");
            return 1;
        }
        std::string method = lua_tostring(lua, 3);
        std::string fullName = fmt::format("{0}.{1}", service->GetName(), method);
        const RpcMethodConfig *methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
        if (methodConfig == nullptr)
        {
            luaL_error(lua, "call [%s] not found", method.c_str());
            return 0;
        }

        const std::string &func = methodConfig->FullName;
        if (!methodConfig->Request.empty())
        {
            const std::string &pb = methodConfig->Request;
            ProtoComponent *messageComponent = App::Inst()->GetMsgComponent();
            std::shared_ptr<Message> message = messageComponent->Read(lua, pb, 4);
            if (message == nullptr)
            {
                luaL_error(lua, "lua call %s request is empty", fullName.c_str());
                return 0;
            }
            if(message->GetTypeName() != methodConfig->Request)
            {
                luaL_error(lua, "lua call %s request type error", fullName.c_str());
                return 0;
            }
            request->WriteMessage(message.get());
        }

        lua_pushthread(lua);
        const std::string &response = methodConfig->Response;
        std::shared_ptr<LuaRpcTaskSource> luaRpcTaskSource
            = std::make_shared<LuaRpcTaskSource>(lua, 0, response);
		{
			request->GetHead().Add("func", func);
			request->GetHead().Add("rpc", luaRpcTaskSource->GetRpcId());
		}
        if (!netMessageComponent->Send(address, request))
        {
            luaL_error(lua, "send request message error");
            return 0;
        }
        long long id = luaRpcTaskSource->GetRpcId();
        return netMessageComponent->AddTask(id, luaRpcTaskSource)->Await();
    }

	int Service::AllotLocation(lua_State *lua)
	{
		Sentry::RpcService* service = UserDataParameter::Read<Sentry::RpcService*>(lua, 1);
		LocationComponent * locationComponent = App::Inst()->GetComponent<LocationComponent>();
		if(lua_isinteger(lua, 2))
		{
			std::string address;
			long long userId = lua_tointeger(lua, 2);
			LocationUnit * locationUnit = locationComponent->GetLocationUnit(userId);
			if(locationUnit != nullptr && locationUnit->Get(service->GetName(), address))
			{
				lua_pushlstring(lua, address.c_str(), address.size());
				return 1;
			}
		}
		std::string address;
		if(locationComponent->AllotLocation(service->GetName(), address))
		{
			lua_pushlstring(lua, address.c_str(), address.size());
			return 1;
		}
		return 0;
	}

    int Service::AddLocation(lua_State *lua)
    {
        return 0;
    }
}