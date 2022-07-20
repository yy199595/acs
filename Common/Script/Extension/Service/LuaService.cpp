//
// Created by yjz on 2022/5/23.
//

#include"LuaService.h"
#include"App/App.h"
#include"Util/StringHelper.h"
#include"Script/Extension/Json/Json.h"
#include"Async/RpcTask/RpcTaskSource.h"
#include"Component/Scene/MessageComponent.h"
#include"Component/Rpc/ServiceRpcComponent.h"
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
		std::shared_ptr<com::Rpc::Request> request(new com::Rpc::Request());
		ServiceComponent* callComponent = UserDataParameter::Read<ServiceComponent*>(lua, 1);
		if (lua_isinteger(lua, 2)) //userId
		{
			userId = lua_tointeger(lua, 2);
			request->set_user_id(userId);
			if(!callComponent->GetAddressProxy().GetAddress(userId, address))
			{
				luaL_error(lua, "not find user address : %lld", userId);
				return 0;
			}
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
		const RpcServiceConfig & rpcServiceConfig = callComponent->GetServiceConfig();
		const RpcInterfaceConfig* rpcInterfaceConfig = rpcServiceConfig.GetConfig(method);
		if (rpcInterfaceConfig == nullptr)
		{
			luaL_error(lua, "call [%s] not found", method.c_str());
			return 0;
		}
		if (!rpcInterfaceConfig->Request.empty())
		{
			MessageComponent * messageComponent = App::Get()->GetMsgComponent();
			std::shared_ptr<Message> message = messageComponent->Read(lua, rpcInterfaceConfig->Request, 4);
			if(message == nullptr)
			{
				luaL_error(lua, "read request paremeter error");
				return 0;
			}
			request->mutable_data()->PackFrom(*message);
		}

		lua_pushthread(lua);
		std::shared_ptr<LuaRpcTaskSource> luaRpcTaskSource(new LuaRpcTaskSource(lua));
		ServiceRpcComponent * rpcHandlerComponent = App::Get()->GetComponent<ServiceRpcComponent>();


		request->set_func(rpcInterfaceConfig->FullName);
		request->set_rpc_id(luaRpcTaskSource->GetRpcId());
		rpcHandlerComponent->AddTask(luaRpcTaskSource);
		if(callComponent->SendRequest(address, request) == XCode::Successful)
		{

		}
		return luaRpcTaskSource->Await();
	}

	int Service::GetAddress(lua_State* lua)
	{
		std::string address;
		ServiceComponent* callComponent = UserDataParameter::Read<ServiceComponent*>(lua, 1);

		if(callComponent->GetAddressProxy().GetAddress(address))
		{
			lua_pushlstring(lua, address.c_str(), address.size());
			return 1;
		}
		return 0;
	}
}