#include"LuaServiceMethod.h"
#include"Lua/Engine/Function.h"
#include"Entity/Unit/App.h"
#include"Lua/Module/LuaModule.h"
#include"Util/Json/Lua/Json.h"
#include"Server/Config/ServiceConfig.h"
#include"Rpc/Lua/LuaServiceTaskSource.h"
#include"Proto/Component/ProtoComponent.h"
#include"Lua/Component/LuaScriptComponent.h"
#include"google/protobuf/util/json_util.h"
namespace Tendo
{

	LuaServiceMethod::LuaServiceMethod(const RpcMethodConfig * config)
		: ServiceMethod(config->Method), mLuaEnv(nullptr), mConfig(config)
	{
		this->mLuaComponent = App::Inst()->GetComponent<LuaScriptComponent>();
		this->mLuaEnv = this->mLuaComponent->GetLuaEnv();
	}

	int LuaServiceMethod::Call(Msg::Packet & message)
	{
		if (lua_pcall(this->mLuaEnv, 1, 2, 0) != 0)
		{
			const char * err = lua_tostring(this->mLuaEnv, -1);
			message.GetHead().Add("error", err);
			LOG_ERROR("call lua " << this->mConfig->FullName << " error=" << err);
			return XCode::CallLuaFunctionFail;
		}
		int code = lua_tointeger(this->mLuaEnv, 1);
		const std::string & res = this->mConfig->Response;
		if (code != XCode::Successful || res.empty())
		{
			return code;
		}
		switch(message.GetProto())
		{
			case Msg::Porto::Json:
			case Msg::Porto::String:
			{
				if (lua_istable(this->mLuaEnv, 3))
				{
					std::string json;
					Lua::RapidJson::Read(this->mLuaEnv, 3, &json);
					message.SetContent(json);
				}
				else if (lua_isstring(this->mLuaEnv, 3))
				{
					size_t len = 0;
					const char* str = lua_tolstring(this->mLuaEnv, 3, &len);
					message.SetContent({ str, len });
				}
			}
				break;
			case Msg::Porto::Protobuf:
			{
				ProtoComponent * protoComponent = App::Inst()->GetProto();
				std::shared_ptr<Message> response = protoComponent->Read(this->mLuaEnv, res, -1);
				if(response == nullptr)
				{
					return XCode::CreateProtoFailure;
				}
				message.WriteMessage(response.get());
			}
				break;
		}
		return XCode::Successful;
	}

	int LuaServiceMethod::CallAsync(Msg::Packet & message)
    {
		std::shared_ptr<Message> response;
		if (!this->mConfig->Response.empty())
		{
			ProtoComponent * protoComponent = App::Inst()->GetProto();
			if(!protoComponent->New(this->mConfig->Response, response))
			{
				return XCode::CreateProtoFailure;
			}
		}
		std::unique_ptr<LuaServiceTaskSource> luaTaskSource =
                std::make_unique<LuaServiceTaskSource>(&message, response);
        Lua::UserDataParameter::Write(this->mLuaEnv, luaTaskSource.get());
        if (lua_pcall(this->mLuaEnv, 3, 1, 0) != 0)
        {           
            message.GetHead().Add("error", lua_tostring(this->mLuaEnv, -1));
            return XCode::CallLuaFunctionFail;
        }
        return luaTaskSource->Await();
    }

	int LuaServiceMethod::Invoke(Msg::Packet & message)
    {
        if (this->mConfig->IsAsync)
        {
            if (!Lua::Function::Get(this->mLuaEnv, "coroutine", "rpc"))
            {
                return XCode::CallLuaFunctionFail;
            }
        }
		const std::string & method = this->mConfig->Method;
		const std::string & module = this->mConfig->Service;
		Lua::LuaModule * luaModule = this->mLuaComponent->GetModule(module);
		if(luaModule == nullptr)
		{
			return XCode::CallServiceNotFound;
		}
		if(!luaModule->GetFunction(method))
		{
			return XCode::CallServiceNotFound;
		}
		lua_createtable(this->mLuaEnv, 0, 3);
		{
			long long userId = 0;
			if (message.GetHead().Get("id", userId))
			{
				lua_pushinteger(this->mLuaEnv, userId);
				lua_setfield(this->mLuaEnv, -2, "userId");
			}
			{
				const std::string& from = message.From();
				lua_pushlstring(this->mLuaEnv, from.c_str(), from.size());
				lua_setfield(this->mLuaEnv, -2, "from");
			}
			{
				switch (message.GetProto())
				{
					case Msg::Porto::Json:
					{
						const std::string& json = message.GetBody();
						Lua::RapidJson::Write(this->mLuaEnv, json);
					}
						break;
					case Msg::Porto::String:
					{
						const std::string& str = message.GetBody();
						lua_pushlstring(this->mLuaEnv, str.c_str(), str.size());
					}
						break;
					case Msg::Porto::Protobuf:
					{
						if (!this->mConfig->Request.empty())
						{
							std::shared_ptr<Message> request;
							ProtoComponent * protoComponent = App::Inst()->GetProto();
							if (!protoComponent->New(this->mConfig->Request, request))
							{
								return false;
							}
							if (!message.ParseMessage(request.get()))
							{
								return XCode::ParseMessageError;
							}
							protoComponent->Write(this->mLuaEnv, *request);
						}
						else
						{
							lua_pushnil(this->mLuaEnv);
						}
					}
						break;
					default:
						return XCode::UnKnowPacket;
				}
				lua_setfield(this->mLuaEnv, -2, "message");
			}
		}

        if (!this->mConfig->IsAsync)
        {
            return this->Call(message);
        }
        return this->CallAsync(message);
    }
}
