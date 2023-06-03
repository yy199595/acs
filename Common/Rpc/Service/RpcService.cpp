#include"RpcService.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Lua/Component/LuaScriptComponent.h"
#include"Proto/Component/ProtoComponent.h"
#include "Util/Json/Lua/Json.h"
#include "Rpc/Lua/LuaServiceTaskSource.h"
#include "Proto/Lua/Message.h"


namespace Tendo
{
	extern std::string GET_FUNC_NAME(const std::string& fullName)
	{
		size_t pos = fullName.find("::");
		return fullName.substr(pos + 2);
	}

	RpcService::RpcService()
		: mMethodRegister(this), mConfig(nullptr)
	{
		this->mLuaModule = nullptr;
	}
	bool RpcService::LateAwake()
	{
		const std::string & name = this->GetName();
		this->mConfig = RpcConfig::Inst()->GetConfig(name);
		LuaScriptComponent * luaComponent = this->GetComponent<LuaScriptComponent>();
		LOG_CHECK_RET_FALSE(ClusterConfig::Inst()->GetServerName(name, this->mCluster));
		if(luaComponent != nullptr)
		{
			this->mLuaModule = luaComponent->LoadModule(name);
		}
		LOG_CHECK_RET_FALSE(this->mConfig);
		LOG_CHECK_RET_FALSE(this->OnInit());
		if(this->mLuaModule != nullptr)
		{
			std::vector<const RpcMethodConfig*> rpcMethodConfigs;
			this->mConfig->GetMethodConfigs(rpcMethodConfigs);
			for (const RpcMethodConfig* rpcMethodConfig: rpcMethodConfigs)
			{
				this->mLuaModule->GetFunction(rpcMethodConfig->Method);
			}
		}
		return true;
	}

	int RpcService::Invoke(const std::string& method, const std::shared_ptr<Msg::Packet>& message)
	{
		if(this->mLuaModule != nullptr && this->mLuaModule->HasFunction(method))
		{
			const RpcMethodConfig* methodConfig = this->mConfig->GetMethodConfig(method);
			if (methodConfig == nullptr)
			{
				return XCode::NotFoundRpcConfig;
			}
			if(!methodConfig->Response.empty())
			{
				ProtoComponent* protoComponent = App::Inst()->GetProto();
				if (!protoComponent->New(methodConfig->Response, message->mBindData))
				{
					return XCode::CreateProtoFailure;
				}
			}
			if (!methodConfig->IsAsync)
			{
				return this->CallLua(methodConfig, *message);
			}
			return this->AwaitCallLua(methodConfig, *message);
		}

		ServiceMethod * target = this->mMethodRegister.GetMethod(method);
		return target == nullptr ? XCode::CallFunctionNotExist : target->Invoke(*message);
	}

	int RpcService::WriterToLua(const RpcMethodConfig * config, Msg::Packet & message)
	{
		lua_State* lua = this->mLuaModule->GetLuaEnv();
		lua_createtable(lua, 0, 3);
		{
			long long userId = 0;
			if (message.ConstHead().Get("id", userId))
			{
				lua_pushinteger(lua, userId);
				lua_setfield(lua, -2, "id");
			}
			{
				const std::string& from = message.From();
				lua_pushlstring(lua, from.c_str(), from.size());
				lua_setfield(lua, -2, "from");
			}
			switch (message.GetProto())
			{
				case Msg::Porto::Json:
					Lua::RapidJson::Write(lua, message.GetBody());
					break;
				case Msg::Porto::String:
				{
					const std::string& str = message.GetBody();
					lua_pushlstring(lua, str.c_str(), str.size());
					break;
				}
				case Msg::Porto::Protobuf:
				{
					if (!config->Request.empty())
					{
						std::shared_ptr<Message> request;
						ProtoComponent* protoComponent = App::Inst()->GetProto();
						if (!protoComponent->New(config->Request, request))
						{
							return XCode::CreateProtoFailure;
						}
						if (!message.ParseMessage(request.get()))
						{
							return XCode::ParseMessageError;
						}
						protoComponent->Write(lua, *request);
					}
				}
					break;
				default:
					return XCode::UnKnowPacket;
			}
			lua_setfield(lua, -2, "message");
		}
		return XCode::Successful;
	}

	int RpcService::CallLua(const RpcMethodConfig * config, Msg::Packet & message)
	{
		this->mLuaModule->GetFunction(config->Method);
		if(this->WriterToLua(config, message) != XCode::Successful)
		{
			return XCode::CallLuaFunctionFail;
		}
		lua_State * lua = this->mLuaModule->GetLuaEnv();
		if (lua_pcall(lua, 1, 2, 0) != LUA_OK)
		{
			const char * err = lua_tostring(lua, -1);
			message.GetHead().Add("error", err);
			return XCode::CallLuaFunctionFail;
		}
		if (lua_istable(lua, -1))
		{
			if (message.mBindData != nullptr)
			{
				MessageEncoder messageEncoder(lua);
				message.SetProto(Msg::Porto::Protobuf);
				if (!messageEncoder.Encode(message.mBindData, -1))
				{
					return XCode::ParseMessageError;
				}
				message.WriteMessage(message.mBindData.get());
				message.mBindData = nullptr;
			}
			else
			{
				message.SetProto(Msg::Porto::Json);
				Lua::RapidJson::Read(lua, -1, message.Body());
			}
		}
		else if (lua_isstring(lua, -1))
		{
			size_t len = 0;
			const char* str = lua_tolstring(lua, -1, &len);
			message.Body()->append(str, len);
			message.SetProto(Msg::Porto::String);
		}
		return (int)luaL_checkinteger(lua, -2);
	}

	int RpcService::AwaitCallLua(const RpcMethodConfig * config, Msg::Packet& message)
	{
		lua_State * lua = this->mLuaModule->GetLuaEnv();
		if (!Lua::Function::Get(lua, "coroutine", "rpc"))
		{
			return XCode::CallLuaFunctionFail;
		}
		this->mLuaModule->GetFunction(config->Method);
		if(this->WriterToLua(config, message) != XCode::Successful)
		{
			return XCode::CallLuaFunctionFail;
		}
		std::unique_ptr<LuaServiceTaskSource> luaTaskSource =
				std::make_unique<LuaServiceTaskSource>(&message);
		Lua::UserDataParameter::Write(lua, luaTaskSource.get());
		if (lua_pcall(lua, 3, 1, 0) != LUA_OK)
		{
			message.GetHead().Add("error", lua_tostring(lua, -1));
			return XCode::CallLuaFunctionFail;
		}
		return luaTaskSource->Await();
	}

	void RpcService::Start()
	{
		this->OnStart();
		if(this->mLuaModule != nullptr)
		{
			this->mLuaModule->Await("OnStart");
		}
	}

	void RpcService::Complete()
	{
		this->OnComplete();
		if(this->mLuaModule != nullptr)
		{
			this->mLuaModule->Await("OnComplete");
		}
	}

	void RpcService::Stop()
	{
		this->OnStop();
		if(this->mLuaModule != nullptr)
		{
			this->mLuaModule->Await("OnStop");
			delete this->mLuaModule;
			this->mLuaModule = nullptr;
		}
	}

	void RpcService::OnHotFix()
	{
		if(this->mLuaModule != nullptr)
		{
			this->mLuaModule->Hotfix();
		}
	}

	void RpcService::OnSecondUpdate(int tick)
	{
		this->OnSecond(tick);
		if(this->mLuaModule != nullptr)
		{
			this->mLuaModule->Update(tick);
		}
	}
}