//
// Created by mac on 2022/5/31.
//

#include"Client.h"
#include"App/App.h"
#include"Json/Lua/Json.h"
#include"String/StringHelper.h"
#include"Timer/ElapsedTimer.h"
#include"Lua/LuaParameter.h"
#include"Lua/LuaWaitTaskSource.h"
#include"Component/ClientComponent.h"
#include"Component/ProtoComponent.h"
using namespace Client;
using namespace Sentry;
namespace Lua
{
	int ClientEx::Call(lua_State* lua)
	{
		lua_pushthread(lua);
		ClientComponent * clientComponent = UserDataParameter::Read<ClientComponent*>(lua, 1);
        if(clientComponent == nullptr)
        {
            luaL_error(lua, "not find ClientComponent");
            return 0;
        }
		std::shared_ptr<Rpc::Data> request(new Rpc::Data());

        request->SetType(Tcp::Type::Request);
        request->SetProto(Tcp::Porto::Protobuf);
        ProtoComponent * messageComponent = App::Get()->GetMsgComponent();
        const std::string func = CommonParameter::Read<std::string>(lua, 2);
		std::shared_ptr<LuaWaitTaskSource> luaWaitTaskSource(new LuaWaitTaskSource(lua));
        if(lua_isstring(lua, 3) && lua_istable(lua, 4))
        {
            const char * type = luaL_checkstring(lua, 3);
            std::shared_ptr<Message> message = messageComponent->Read(lua, type, 4);
            if(message == nullptr)
            {
                LOG_ERROR("new proto message " << type << " error");
                return 0;
            }
            request->WriteMessage(message.get());
        }
        request->GetHead().Add("func", func);
		TaskComponent * taskComponent = App::Get()->GetTaskComponent();
		taskComponent->Start([request, func, clientComponent, luaWaitTaskSource]()
		{
			ElapsedTimer elapsedTimer;
			std::shared_ptr<Rpc::Data> response = clientComponent->Call(request);
			if(response == nullptr)
			{
				luaWaitTaskSource->SetResult(XCode::CallTimeout, nullptr);
				return;
			}
			std::shared_ptr<Message> message; //TODO
            XCode code = response->GetCode(XCode::Failure);
            if(code == XCode::Successful)
            {

            }
			luaWaitTaskSource->SetResult(code, message);
			LOG_DEBUG("client call " << func << " user time = [" << elapsedTimer.GetMs() << "ms]");
		});
		return luaWaitTaskSource->Await();
	}

    int ClientEx::Auth(lua_State *lua)
    {
        lua_pushthread(lua);
        ClientComponent *clientComponent = UserDataParameter::Read<ClientComponent *>(lua, 1);
        if (clientComponent == nullptr)
        {
            luaL_error(lua, "not find ClientComponent");
            return 0;
        }
        std::shared_ptr<Rpc::Data> request(new Rpc::Data());

        request->SetType(Tcp::Type::Auth);
        std::string token(luaL_checkstring(lua, 2));
        request->GetHead().Add("token", token);
        std::shared_ptr<LuaWaitTaskSource> luaWaitTaskSource(new LuaWaitTaskSource(lua));

        TaskComponent *taskComponent = App::Get()->GetTaskComponent();
        taskComponent->Start([request, clientComponent, luaWaitTaskSource]() {
            XCode code = XCode::Failure;
            std::shared_ptr<Rpc::Data> response = clientComponent->Call(request);
            if(response != nullptr)
            {
                code = response->GetCode(XCode::Failure);
            }
            luaWaitTaskSource->SetResult(code, nullptr);
        });
        return luaWaitTaskSource->Await();
    }

	int ClientEx::StartConnect(lua_State* lua)
	{
		std::string ip;
		unsigned short port;
		lua_pushthread(lua);
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