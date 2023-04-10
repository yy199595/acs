//
// Created by mac on 2022/5/31.
//

#include"Client.h"
#include"XCode/XCode.h"
#include"Entity/App/App.h"
#include"Server/Config/ServiceConfig.h"
#include"Util/String/StringHelper.h"
#include"Timer/Timer/ElapsedTimer.h"
#include"Script/Lua/LuaParameter.h"
#include"Component/ClientComponent.h"
#include"Proto/Component/ProtoComponent.h"
using namespace Client;
using namespace Tendo;
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
			request->SetType(Msg::Type::Request);
			request->SetProto(Msg::Porto::Protobuf);
		}

        ProtoComponent * messageComponent = App::Inst()->GetMsgComponent();
        const std::string func = CommonParameter::Read<std::string>(lua,2);
        const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(func);
        if (methodConfig == nullptr)
        {
            LOG_ERROR("call [" << func << "] not exist");
            return 0;
        }
       

		//std::shared_ptr<LuaWaitTaskSource> luaWaitTaskSource(new LuaWaitTaskSource(lua));
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
			request->SetProto(Msg::Porto::String);
			request->SetContent(lua_tostring(lua, 3));
		}
        int rpcId = 0;
        request->GetHead().Add("func", func);
        if (!clientComponent->Send(session, request, rpcId))
        {
            lua_pushinteger(lua, XCode::NetWorkError);
            return 1;
        }
        const std::string& response = methodConfig->Response;
        std::shared_ptr<LuaRpcTaskSource> luaRpcTaskSource
            = std::make_shared<LuaRpcTaskSource>(lua, rpcId, response);
        return clientComponent->AddTask(rpcId, luaRpcTaskSource)->Await();	
	}

    int ClientEx::New(lua_State* lua)
    {
        std::string ip;
        unsigned short port;
		const std::string address = CommonParameter::Read<std::string>(lua, 1);
		ClientComponent* clientComponent = App::Inst()->GetComponent<ClientComponent>();
        if (!Helper::Str::SplitAddr(address, ip, port))
        {
            luaL_error(lua, "parse ip address [%s] error", address.c_str());
            return 0;
        }
		lua_pushinteger(lua, clientComponent->New(ip, port));
		return 1;
    }
}