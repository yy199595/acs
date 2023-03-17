//
// Created by mac on 2022/5/31.
//

#include"Client.h"
#include"App/App.h"
#include"Json/Lua/Json.h"
#include"Config/ServiceConfig.h"
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
		int session = luaL_checkinteger(lua, 1);
        ClientComponent* clientComponent = App::Inst()->GetComponent<ClientComponent>();
        if(clientComponent == nullptr)
        {
            luaL_error(lua, "not find ClientComponent");
            return 0;
        }
		std::shared_ptr<Rpc::Packet> request(new Rpc::Packet());
		{
			request->SetType(Tcp::Type::Request);
			request->SetProto(Tcp::Porto::Protobuf);
		}

        ProtoComponent * messageComponent = App::Inst()->GetMsgComponent();
        const std::string func = CommonParameter::Read<std::string>(lua,2);
        const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(func);
        if (methodConfig == nullptr)
        {
            LOG_ERROR("call [" << func << "] not exist");
            return 0;
        }
		std::shared_ptr<LuaWaitTaskSource> luaWaitTaskSource(new LuaWaitTaskSource(lua));
        if (lua_istable(lua, 3) && !methodConfig->Request.empty())
        {
            const std::string& name = methodConfig->Request;
            std::shared_ptr<Message> message = messageComponent->Read(lua, name, 3);
            if (message == nullptr || !request->WriteMessage(message.get()))
            {
                LOG_ERROR("write request message error : [" << func << "]");
                return 0;
            }
        }
		else if(lua_isstring(lua, 3))
		{
			request->SetProto(Tcp::Porto::String);
			request->SetContent(lua_tostring(lua, 3));
		}
        request->GetHead().Add("func", func);
		TaskComponent * taskComponent = App::Inst()->GetTaskComponent();
		taskComponent->Start([session, request, methodConfig, clientComponent,
							  luaWaitTaskSource, messageComponent]()
		{
			ElapsedTimer elapsedTimer;
			std::shared_ptr<Rpc::Packet> response = clientComponent->Call(session, request);
			if(response == nullptr)
			{
				luaWaitTaskSource->SetResult(XCode::CallTimeout, nullptr);
				return;
			}
            std::shared_ptr<Message> message;
            int code = response->GetCode();
            if(code == XCode::Successful)
            {
                if (!methodConfig->Response.empty())
                {
                    const std::string& name = methodConfig->Response;
                    message = messageComponent->New(name);
                    response->ParseMessage(message.get());
                }                  
            }
			luaWaitTaskSource->SetResult(code, message);
			if(elapsedTimer.GetSecond() >= 10)
			{
				LOG_ERROR("client call " << methodConfig->FullName <<
					" user time = [" << elapsedTimer.GetSecond() << "s]");
			}
		});
		return luaWaitTaskSource->Await();
	}

    int ClientEx::New(lua_State* lua)
    {
        std::string ip;
        unsigned short port;
		const std::string address = CommonParameter::Read<std::string>(lua, 1);
		ClientComponent* clientComponent = App::Inst()->GetComponent<ClientComponent>();
        if (!Helper::Str::SplitAddress(address, ip, port))
        {
            luaL_error(lua, "parse ip address [%s] error", address.c_str());
            return 0;
        }
		lua_pushinteger(lua, clientComponent->New(ip, port));
		return 1;
    }
}