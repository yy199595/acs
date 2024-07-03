#include"RpcService.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Lua/Component/LuaComponent.h"
#include "Rpc/Lua/LuaServiceTaskSource.h"
#include"Yyjson/Lua/ljson.h"
#include "Proto/Lua/Message.h"

#include"Core/System/System.h"
namespace joke
{
	extern std::string GET_FUNC_NAME(const std::string& fullName)
	{
		size_t pos = fullName.find("::");
		return fullName.substr(pos + 2);
	}

	RpcService::RpcService()
		: mMethodRegister(this)
	{
		this->mProto = nullptr;
		this->mLuaModule = nullptr;
	}


	bool RpcService::LateAwake()
	{
		const std::string & name = this->GetName();
		LOG_CHECK_RET_FALSE(RpcConfig::Inst()->HasService(name))
		if(!ClusterConfig::Inst()->GetServerName(name, this->mCluster))
		{
			LOG_WARN("{} not find cluster config", name);
		}
		LuaComponent * luaComponent = this->GetComponent<LuaComponent>();
		if(luaComponent != nullptr)
		{
			this->mLuaModule = luaComponent->LoadModule(name);
			if(this->mLuaModule != nullptr)
			{
				this->mLuaModule->SetMember("__server", this->mCluster);
			}
		}
		LOG_CHECK_RET_FALSE(this->OnInit());
		this->mProto = this->mApp->GetProto();
		if(this->mLuaModule != nullptr)
		{		
			int code = this->mLuaModule->Call("Awake");
			return code != XCode::CallLuaFunctionFail;
		}
		return true;
	}

	int RpcService::Invoke(const RpcMethodConfig * methodConfig, rpc::Packet * message)
	{
		const std::string & method = methodConfig->Method;
		if(this->mLuaModule != nullptr && this->mLuaModule->HasFunction(method))
		{
			message->TempHead().Add("pb", methodConfig->Response);
			if (!methodConfig->IsAsync)
			{
				return this->CallLua(methodConfig, *message);
			}
			return this->AwaitCallLua(methodConfig, *message);
		}
		ServiceMethod * target = this->mMethodRegister.GetMethod(method);
		return target == nullptr ? XCode::CallFunctionNotExist : target->Invoke(*message);
	}

	int RpcService::WriterToLua(const RpcMethodConfig * config, rpc::Packet & message)
	{
		lua_State* lua = this->mLuaModule->GetLuaEnv();
		lua_pushlstring(lua, config->Method.c_str(), config->Method.size());
		lua_createtable(lua, 0, 2);
		{
			tcp::IHeader::WriteLua(lua, message.GetHead());
			{
				lua_setfield(lua, -2, "head");
			}
			switch (message.GetProto())
			{
				case rpc::Porto::None:
					return XCode::Ok;
				case rpc::Porto::Json:
				{
					const char * data = message.GetBody().c_str();
					const size_t size = message.GetBody().size();
					lua::yyjson::write(lua, data, size);
					break;
				}
				case rpc::Porto::String:
				{
					const std::string& str = message.GetBody();
					lua_pushlstring(lua, str.c_str(), str.size());
					break;
				}
				case rpc::Porto::Protobuf:
				{
					if (!config->Request.empty())
					{
						pb::Message * request = this->mProto->Temp(config->Request);
						{
							if (request == nullptr)
							{
								return XCode::CreateProtoFailure;
							}
							if (!message.ParseMessage(request))
							{
								return XCode::ParseMessageError;
							}
							this->mProto->Write(lua, *request);
						}
						request->Clear();
					}
					break;
				}
				default:
					return XCode::UnKnowPacket;
			}
			lua_setfield(lua, -2, "data");
		}
		return XCode::Ok;
	}

	int RpcService::CallLua(const RpcMethodConfig * config, rpc::Packet & message)
	{
		this->mLuaModule->GetMetaFunction("__Invoke");
		lua_State* lua = this->mLuaModule->GetLuaEnv();
		int code = this->WriterToLua(config, message);
		if(code != XCode::Ok)
		{
			return code;
		}
		if (lua_pcall(lua, 3, 2, 0) != LUA_OK)
		{
			std::string err;
			this->mLuaModule->SpliteError(err);
			message.GetHead().Add("error", err);
			LOG_ERROR("{} {}", config->FullName, err);
			return XCode::CallLuaFunctionFail;
		}
		message.SetProto(rpc::Porto::None);
		if (lua_istable(lua, -1))
		{
			std::string pb;
			if(message.TempHead().Del("pb", pb))
			{
				MessageEncoder messageEncoder(lua);
				message.SetProto(rpc::Porto::Protobuf);
				pb::Message * data = this->mProto->Temp(pb);
				if(data == nullptr)
				{
					return XCode::CreateProtoFailure;
				}
				if (!messageEncoder.Encode(*data, -1))
				{
					return XCode::ParseMessageError;
				}
				message.WriteMessage(data);
			}
			else
			{
				message.SetProto(rpc::Porto::Json);
				lua::yyjson::read(lua, -1, *message.Body());
			}
		}
		else if (lua_isstring(lua, -1))
		{
			size_t len = 0;
			message.Body()->clear();
			const char* str = lua_tolstring(lua, -1, &len);
			message.Body()->append(str, len);
			message.SetProto(rpc::Porto::String);
		}
		return (int)luaL_checkinteger(lua, -2);
	}

	int RpcService::AwaitCallLua(const RpcMethodConfig * config, rpc::Packet& message)
	{
		this->mLuaModule->GetMetaFunction("__Call");
		lua_State* lua = this->mLuaModule->GetLuaEnv();
		if(this->WriterToLua(config, message) != XCode::Ok)
		{
			return XCode::CallLuaFunctionFail;
		}
		std::unique_ptr<LuaServiceTaskSource> luaTaskSource =
				std::make_unique<LuaServiceTaskSource>(&message);

		Lua::UserDataParameter::Write(lua, luaTaskSource.get());
		if (lua_pcall(lua, 4, 1, 0) != LUA_OK)
		{
			std::string err;
			this->mLuaModule->SpliteError(err);
			LOG_ERROR("{} {}", config->FullName, err);
			return XCode::CallLuaFunctionFail;
		}
		return luaTaskSource->Await();
	}

	void RpcService::Start()
	{
		this->OnStart();
		IF_NOT_NULL_CALL(this->mLuaModule, Await, "OnStart")
	}

	void RpcService::Complete()
	{
		this->OnComplete();
		IF_NOT_NULL_CALL(this->mLuaModule, Await, "OnComplete")
	}

	void RpcService::OnAppStop()
	{
		this->OnStop();
		if(this->mLuaModule != nullptr)
		{
			this->mLuaModule->Await("OnStop");
			this->mLuaModule = nullptr;
		}
	}
}