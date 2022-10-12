//
// Created by yjz on 2022/5/23.
//

#include"LuaService.h"
#include"App/App.h"
#include"Json/Lua/Json.h"
#include"String/StringHelper.h"
#include"Async/RpcTaskSource.h"
#include"Component/ProtoComponent.h"
#include"Component/InnerNetMessageComponent.h"
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
        long long userId = 0;
        Sentry::Service *service = UserDataParameter::Read<Sentry::Service *>(lua, 1);
        InnerNetMessageComponent *netMessageComponent = App::Get()->GetComponent<InnerNetMessageComponent>();
        if (netMessageComponent == nullptr)
        {
            luaL_error(lua, "not find InnerNetMessageComponent");
            return 0;
        }
        if (lua_isinteger(lua, 2)) //userId
        {
            userId = lua_tointeger(lua, 2);
            //TODO
        }
        else if (lua_isstring(lua, 2)) //address
        {
            address.append(lua_tostring(lua, 2));
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
        const RpcServiceConfig &rpcServiceConfig = service->GetServiceConfig();
        const RpcMethodConfig *methodConfig = rpcServiceConfig.GetConfig(method);
        if (methodConfig == nullptr)
        {
            luaL_error(lua, "call [%s] not found", method.c_str());
            return 0;
        }
        std::shared_ptr<Rpc::Data> request(new Rpc::Data());

        request->SetType(Tcp::Type::Request);
        request->SetProto(Tcp::Porto::Protobuf);
        const std::string &func = methodConfig->FullName;
        if (!methodConfig->Request.empty())
        {
            const std::string &pb = methodConfig->Request;
            ProtoComponent *messageComponent = App::Get()->GetMsgComponent();
            std::shared_ptr<Message> message = messageComponent->Read(lua, pb, 4);
            if (message == nullptr)
            {
                luaL_error(lua, "read request paremeter error");
                return 0;
            }
            request->WriteMessage(message.get());
        }

        lua_pushthread(lua);
        const std::string &response = methodConfig->Response;
        std::shared_ptr<LuaRpcTaskSource> luaRpcTaskSource
            = std::make_shared<LuaRpcTaskSource>(lua, 0, response);

        request->GetHead().Add("func", func);
        request->GetHead().Add("rpc", luaRpcTaskSource->GetRpcId());
        if (!netMessageComponent->Send(address, request))
        {
            luaL_error(lua, "send request message error");
            return 0;
        }
        return netMessageComponent->AddTask(luaRpcTaskSource)->Await();
    }

	int Service::AllotLocation(lua_State *lua)
	{
		std::string address;
		Sentry::Service* service = UserDataParameter::Read<Sentry::Service*>(lua, 1);
        if(lua_isinteger(lua, 2))
        {
            long long userId = lua_tointeger(lua, 2);
            if(!service->AllotLocation(userId, address))
            {
                return 0;
            }
        }
		else if(!service->AllotLocation(address))
		{
			return 0;
		}
        lua_pushstring(lua, address.c_str());
        return 1;
	}

    int Service::AddLocation(lua_State *lua)
    {
        long long unitId = 0;
        Sentry::Service* service = UserDataParameter::Read<Sentry::Service*>(lua, 1);
        const char * address = luaL_checkstring(lua, 2);
        if(lua_isinteger(lua, 3) && service != nullptr)
        {
            unitId = lua_tointeger(lua, 2);
            //TODO
            return 0;
        }
        if(service != nullptr)
        {
            service->AddLocation(address);
        }
        return 0;
    }
}