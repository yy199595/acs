//
// Created by mac on 2022/5/31.
//

#include"Client.h"
#include"App/App.h"
#include"Script/Extension/Json/Json.h"
#include"Util/StringHelper.h"
#include"Other/ElapsedTimer.h"
#include"Script/LuaParameter.h"
#include"Component/ClientComponent.h"
#include "Async/Lua/LuaWaitTaskSource.h"
#include"Component/Scene/ProtoComponent.h"
using namespace Client;
using namespace Sentry;
namespace Lua
{
	int ClientEx::Call(lua_State* lua)
	{
		lua_pushthread(lua);
		ClientComponent * clientComponent = UserDataParameter::Read<ClientComponent*>(lua, 1);
		std::shared_ptr<c2s::rpc::request> request(new c2s::rpc::request());
		const std::string func = CommonParameter::Read<std::string>(lua, 2);
		std::shared_ptr<LuaWaitTaskSource> luaWaitTaskSource(new LuaWaitTaskSource(lua));
        ProtoComponent * messageComponent = App::Get()->GetMsgComponent();
        if(lua_isstring(lua, 3) && lua_istable(lua, 4))
        {
            const char * type = luaL_checkstring(lua, 3);
            std::shared_ptr<Message> message = messageComponent->Read(lua, type, 4);
            if(message == nullptr)
            {
                LOG_ERROR("new proto message " << type << " error");
                return 0;
            }
            request->mutable_data()->PackFrom(*message);
        }

		request->set_method_name(func);
		TaskComponent * taskComponent = App::Get()->GetTaskComponent();
		taskComponent->Start([request, func, clientComponent, luaWaitTaskSource]()
		{
			ElapsedTimer elapsedTimer;
			std::shared_ptr<c2s::rpc::response> response = clientComponent->Call(request);
			if(response == nullptr)
			{
				luaWaitTaskSource->SetResult(XCode::CallTimeout, nullptr);
				return;
			}
			std::shared_ptr<Message> message;
			XCode code = (XCode)response->code();
			if(code == XCode::Successful && response->has_data())
			{
				message = App::Get()->GetMsgComponent()->New(response->data());
			}
			luaWaitTaskSource->SetResult(code, message);
			LOG_DEBUG("client call " << func << " user time = [" << elapsedTimer.GetMs() << "ms]");
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
		return clientComponent->StartConnect(ip, port);
	}
}