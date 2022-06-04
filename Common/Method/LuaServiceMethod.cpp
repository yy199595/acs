#include"LuaServiceMethod.h"
#include"Script/Function.h"
#include"App/App.h"
#include"Global/ServiceConfig.h"
#include"Async/LuaServiceTaskSource.h"
#include"Component/Scene/MessageComponent.h"
#include<google/protobuf/util/json_util.h>
#include<Script/Extension/Coroutine/LuaCoroutine.h>
namespace Sentry
{

	LuaServiceMethod::LuaServiceMethod(const std::string& service, const std::string& func, lua_State* lua)
		: ServiceMethod(func), mLuaEnv(lua)
	{
		this->mFunction = func;
		this->mServiceCompoent = App::Get()->GetService(service);
		this->mMsgComponent = App::Get()->GetComponent<MessageComponent>();
	}

	XCode LuaServiceMethod::Call(long long id, std::shared_ptr<Message> request, com::Rpc::Response & response)
	{
		const RpcServiceConfig & rpcServiceConfig = this->mServiceCompoent->GetServiceConfig();
		const RpcInterfaceConfig * protoConfig = rpcServiceConfig.GetConfig(this->mFunction);
		if(protoConfig == nullptr || !Lua::Function::Get(this->mLuaEnv,
			protoConfig->Service.c_str(), protoConfig->Method.c_str()))
		{
			return XCode::NotFoundRpcConfig;
		}
		int count = 1;
		lua_pushinteger(this->mLuaEnv, id);
		if(request != nullptr)
		{
			count++;
			this->mMsgComponent->Write(this->mLuaEnv, *request);
		}
		if (lua_pcall(this->mLuaEnv, count, 2, 0) != 0)
		{
			response.set_error_str(lua_tostring(this->mLuaEnv, -1));
			LOG_ERROR("[" << protoConfig->FullName << "] " << response.error_str());
			return XCode::CallLuaFunctionFail;
		}
		const std::string & res = protoConfig->Response;
		XCode code = (XCode)lua_tointeger(this->mLuaEnv, -2);
		if (code != XCode::Successful || res.empty())
		{
			return code;
		}
		std::shared_ptr<Message> message = this->mMsgComponent->Read(this->mLuaEnv, res, -1);
		if(message == nullptr)
		{
			return XCode::CreateProtoFailure;
		}
		response.mutable_data()->PackFrom(*message);
		return XCode::Successful;
	}

	XCode LuaServiceMethod::CallAsync(long long id, std::shared_ptr<Message> message, com::Rpc::Response & response)
	{
		const RpcServiceConfig & rpcServiceConfig = this->mServiceCompoent->GetServiceConfig();
		const RpcInterfaceConfig * protoConfig = rpcServiceConfig.GetConfig(this->mFunction);

		std::shared_ptr<LuaServiceTaskSource> luaTaskSource(new LuaServiceTaskSource(this->mLuaEnv));

		if(!Lua::Function::Get(this->mLuaEnv, "Service", "Call"))
		{
			return XCode::Failure;
		}

		if(protoConfig == nullptr || !Lua::Function::Get(this->mLuaEnv,
			protoConfig->Service.c_str(), protoConfig->Method.c_str()))
		{
			return XCode::NotFoundRpcConfig;
		}
		lua_pushinteger(this->mLuaEnv, id);
		this->mMsgComponent->Write(this->mLuaEnv, *message);
		Lua::UserDataParameter::Write(this->mLuaEnv, luaTaskSource);
		if (lua_pcall(this->mLuaEnv, 4, 1, 0) != 0)
		{
			response.set_error_str(lua_tostring(this->mLuaEnv, -1));
			LOG_ERROR("call lua " << protoConfig->FullName << " " << response.error_str());
			return XCode::CallLuaFunctionFail;
		}
		XCode code = luaTaskSource->Await();
		if(code != XCode::Successful)
		{
			return code;
		}
		int ref = luaTaskSource->GetTable();
		if(!protoConfig->Response.empty() && ref != 0)
		{
			lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, ref);
			std::shared_ptr<Message> message = this->mMsgComponent->Read(
				this->mLuaEnv, protoConfig->Response, -1);
			if (message == nullptr)
			{
				return XCode::JsonCastProtoFailure;
			}
			response.mutable_data()->PackFrom(*message);
		}
		return XCode::Successful;
	}

	XCode LuaServiceMethod::Invoke(const com::Rpc_Request& request, com::Rpc_Response& response)
	{
		const RpcServiceConfig & rpcServiceConfig = this->mServiceCompoent->GetServiceConfig();
		const RpcInterfaceConfig * protoConfig = rpcServiceConfig.GetConfig(this->mFunction);
		if (protoConfig == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		std::shared_ptr<Message> message;
		if (request.has_data())
		{
			message = this->mMsgComponent->New(request.data());
			if(message == nullptr)
			{
				return XCode::ProtoCastJsonFailure;
			}
		}
		if(!protoConfig->IsAsync)
		{
			return this->Call(request.user_id(), message, response);
		}
		return this->CallAsync(request.user_id(), message, response);
	}
}
