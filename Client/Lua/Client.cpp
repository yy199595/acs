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
        ClientComponent* clientComponent = App::Inst()->GetComponent<ClientComponent>();
        if(clientComponent == nullptr)
        {
            luaL_error(lua, "not find ClientComponent");
            return 0;
        }
		std::shared_ptr<Rpc::Packet> request(new Rpc::Packet());

        request->SetType(Tcp::Type::Request);
        request->SetProto(Tcp::Porto::Protobuf);
        ProtoComponent * messageComponent = App::Inst()->GetMsgComponent();
        const std::string func = CommonParameter::Read<std::string>(lua, 1);
        const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(func);
        if (methodConfig == nullptr)
        {
            LOG_ERROR("call [" << func << "] not exist");
            return 0;
        }
		std::shared_ptr<LuaWaitTaskSource> luaWaitTaskSource(new LuaWaitTaskSource(lua));
        if (lua_istable(lua, 2) && !methodConfig->Request.empty())
        {
            const std::string& name = methodConfig->Request;
            std::shared_ptr<Message> message = messageComponent->Read(lua, name, 2);
            if (message == nullptr || !request->WriteMessage(message.get()))
            {
                LOG_ERROR("write request message error : [" << func << "]");
                return 0;
            }
        }
        request->GetHead().Add("func", func);
		TaskComponent * taskComponent = App::Inst()->GetTaskComponent();
		taskComponent->Start([request, methodConfig, clientComponent, luaWaitTaskSource, messageComponent]()
		{
			ElapsedTimer elapsedTimer;
			std::shared_ptr<Rpc::Packet> response = clientComponent->Call(request);
			if(response == nullptr)
			{
				luaWaitTaskSource->SetResult(XCode::CallTimeout, nullptr);
				return;
			}
            std::shared_ptr<Message> message;
            XCode code = response->GetCode();
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
        lua_pushthread(lua);
        ClientComponent* clientComponent = App::Inst()->GetComponent<ClientComponent>();
        const std::string address = CommonParameter::Read<std::string>(lua, 1);
        if (!Helper::String::ParseIpAddress(address, ip, port))
        {
            luaL_error(lua, "parse ip address [%s] error", address.c_str());
            return 0;
        }
        std::string token(luaL_checkstring(lua, 2));
        std::shared_ptr<Rpc::Packet> request(new Rpc::Packet());
        {
            request->SetType(Tcp::Type::Auth);
            request->GetHead().Add("token", token);
        }
        clientComponent->New(ip, port);
        TaskComponent* taskComponent = App::Inst()->GetTaskComponent();
        std::shared_ptr<LuaWaitTaskSource> luaWaitTaskSource(new LuaWaitTaskSource(lua));

        taskComponent->Start([request, clientComponent, luaWaitTaskSource]()
            {              
                std::shared_ptr<Rpc::Packet> response = clientComponent->Call(request);
                XCode code = response != nullptr ? response->GetCode(XCode::Failure) : XCode::Failure;
                
                luaWaitTaskSource->SetResult<bool>(code == XCode::Successful);
            });
        return luaWaitTaskSource->Await();
    }
}