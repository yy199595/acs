//
// Created by mac on 2022/5/31.
//

#include"Client.h"
#include"App/App.h"
#include"Script/LuaParameter.h"
#include "Async/Lua/LuaWaitTaskSource.h"
#include"Component/ClientComponent.h"
#include"Pool/MessagePool.h"
#include"Util/StringHelper.h"
using namespace Client;
using namespace Sentry;
namespace Lua
{
	int ClientEx::Call(lua_State* lua)
	{
		lua_pushthread(lua);
		ClientComponent * clientComponent = UserDataParameter::Read<ClientComponent*>(lua, 1);
		std::string func = CommonParameter::Read<std::string>(lua, 2);
		std::shared_ptr<c2s::Rpc::Request> request(new c2s::Rpc::Request());
		std::shared_ptr<LuaWaitTaskSource> luaWaitTaskSource(new LuaWaitTaskSource(lua));
		if(lua_isuserdata(lua, 3))
		{
			std::shared_ptr<Message> message = UserDataParameter::Read<std::shared_ptr<Message>>(lua, 3);
			request->mutable_data()->PackFrom(*message);
		}
		request->set_method_name(func);
		TaskComponent * taskComponent = App::Get()->GetTaskComponent();
		taskComponent->Start([request, clientComponent, luaWaitTaskSource]()
		{
			std::shared_ptr<c2s::Rpc::Response> response = clientComponent->Call(request);
			if(response == nullptr)
			{
				luaWaitTaskSource->SetResult(XCode::CallTimeout, nullptr);
				return;
			}
			std::shared_ptr<Message> message;
			XCode code = (XCode)response->code();
			if(code == XCode::Successful && response->has_data())
			{
				message = Helper::Proto::NewByData(response->data());
			}
			luaWaitTaskSource->SetResult(code, message);
		});
		return luaWaitTaskSource->Await();
	}

	int ClientEx::StartConnect(lua_State* lua)
	{
		std::string ip;
		unsigned short port;
		lua_pushthread(lua);
		TaskComponent * taskComponent = App::Get()->GetTaskComponent();
		ClientComponent * clientComponent = UserDataParameter::Read<ClientComponent*>(lua, 1);
		const std::string address = CommonParameter::Read<std::string>(lua, 2);
		if(!Helper::String::ParseIpAddress(address, ip, port))
		{
			luaL_error(lua, "parse ip address [%s] error", address.c_str());
			return 0;
		}
		std::shared_ptr<LuaWaitTaskSource> luaWaitTaskSource(new LuaWaitTaskSource(lua));

		taskComponent->Start([clientComponent, ip, port, luaWaitTaskSource]()
		{
			luaWaitTaskSource->SetResult(clientComponent->StartConnect(ip, port));
		});
		return luaWaitTaskSource->Await();
	}
}