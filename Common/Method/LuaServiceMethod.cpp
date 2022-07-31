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

	LuaServiceMethod::LuaServiceMethod(const RpcInterfaceConfig * config, lua_State* lua)
		: ServiceMethod(config->Method), mConfig(config), mLuaEnv(lua)
	{
		this->mMsgComponent = App::Get()->GetComponent<MessageComponent>();
	}

	XCode LuaServiceMethod::Call(int count, com::rpc::response & response)
	{
		if (lua_pcall(this->mLuaEnv, count, 2, 0) != 0)
		{
			response.set_error_str(lua_tostring(this->mLuaEnv, -1));
			LOG_ERROR("[" << this->mConfig->FullName << "] " << response.error_str());
			return XCode::CallLuaFunctionFail;
		}
		const std::string & res = this->mConfig->Response;
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

	XCode LuaServiceMethod::CallAsync(int count, com::rpc::response & response)
	{
		std::shared_ptr<LuaServiceTaskSource> luaTaskSource(new LuaServiceTaskSource(this->mLuaEnv));

		Lua::UserDataParameter::Write(this->mLuaEnv, luaTaskSource);
		if (lua_pcall(this->mLuaEnv, count + 2, 1, 0) != 0)
		{
			response.set_error_str(lua_tostring(this->mLuaEnv, -1));
			LOG_ERROR("call lua " << this->mConfig->FullName << " " << response.error_str());
			return XCode::CallLuaFunctionFail;
		}
		XCode code = luaTaskSource->Await();
		if(code != XCode::Successful)
		{
			return code;
		}
		if(!this->mConfig->Response.empty() && luaTaskSource->GetRef())
		{
			std::shared_ptr<Message> message = this->mMsgComponent->Read(
				this->mLuaEnv, this->mConfig->Response, -1);
			if (message == nullptr)
			{
				return XCode::JsonCastProtoFailure;
			}
			response.mutable_data()->PackFrom(*message);
		}
		return XCode::Successful;
	}

	XCode LuaServiceMethod::Invoke(const com::rpc::request& request, com::rpc::response& response)
	{
        if(this->mConfig->IsAsync && !Lua::Function::Get(this->mLuaEnv, "RpcCall"))
        {
            return XCode::CallLuaFunctionFail;
        }
        const char * tab = this->mConfig->Service.c_str();
        const char * method = this->mConfig->Method.c_str();
        if(!Lua::Function::Get(this->mLuaEnv, tab, method))
        {
            return XCode::NotFoundRpcConfig;
        }

		std::shared_ptr<Message> message;
        if (!this->mConfig->Request.empty())
		{
            if(!request.has_data())
            {
                return XCode::CallArgsError;
            }
			message = this->mMsgComponent->New(request.data());
			if(message == nullptr)
			{
				return XCode::CallArgsError;
			}
		}
        int count = 1;
        lua_pushinteger(this->mLuaEnv, request.user_id());
        if(message != nullptr)
        {
            count++;
            this->mMsgComponent->Write(this->mLuaEnv, *message);
        }
		if(!this->mConfig->IsAsync)
		{
			return this->Call(count, response);
		}
		return this->CallAsync(count, response);
	}
}
