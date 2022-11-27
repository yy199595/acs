#include"LuaServiceMethod.h"
#include"Lua/Function.h"
#include"App/App.h"
#include"Config/ServiceConfig.h"
#include"Lua/LuaServiceTaskSource.h"
#include"Component/ProtoComponent.h"
#include"Component/LuaScriptComponent.h"
#include"google/protobuf/util/json_util.h"
namespace Sentry
{

	LuaServiceMethod::LuaServiceMethod(const RpcMethodConfig * config)
		: ServiceMethod(config->Method), mLuaEnv(nullptr), mConfig(config)
	{
		this->mMsgComponent = App::Inst()->GetComponent<ProtoComponent>();
		this->mLuaComponent = App::Inst()->GetComponent<LuaScriptComponent>();
		this->mLuaEnv = this->mLuaComponent->GetLuaEnv();
	}

	XCode LuaServiceMethod::Call(int count, Rpc::Packet & message)
	{
		if (lua_pcall(this->mLuaEnv, count, 2, 0) != 0)
		{
            message.GetHead().Add("error", lua_tostring(this->mLuaEnv, -1));
			return XCode::CallLuaFunctionFail;
		}
		const std::string & res = this->mConfig->Response;
		XCode code = (XCode)lua_tointeger(this->mLuaEnv, -2);
		if (code != XCode::Successful || res.empty())
		{
			return code;
		}
		std::shared_ptr<Message> response = this->mMsgComponent->Read(this->mLuaEnv, res, -1);
		if(response == nullptr)
		{
			return XCode::CreateProtoFailure;
		}
        message.WriteMessage(response.get());
		return XCode::Successful;
	}

	XCode LuaServiceMethod::CallAsync(int count, Rpc::Packet & message)
    {
        auto * luaTaskSource = new LuaServiceTaskSource(this->mLuaEnv);
        Lua::UserDataParameter::Write(this->mLuaEnv, luaTaskSource);
        if (lua_pcall(this->mLuaEnv, count + 2, 1, 0) != 0)
        {
            delete luaTaskSource;
            message.GetHead().Add("error", lua_tostring(this->mLuaEnv, -1));
            return XCode::CallLuaFunctionFail;
        }
        XCode code = luaTaskSource->Await();

        if (code != XCode::Successful)
        {
            delete luaTaskSource;
			return code;
        }
        if (!this->mConfig->Response.empty() && luaTaskSource->GetRef())
        {
            std::shared_ptr<Message> response = this->mMsgComponent->Read(
				this->mLuaEnv, this->mConfig->Response, -1);
            if (response == nullptr)
            {
                delete luaTaskSource;
                return XCode::JsonCastProtoFailure;
            }
            if (message.WriteMessage(response.get()))
            {
                delete luaTaskSource;
                return XCode::SerializationFailure;
            }
        }
        delete luaTaskSource;
        return XCode::Successful;
    }

	XCode LuaServiceMethod::Invoke(Rpc::Packet & message)
    {
        if (this->mConfig->IsAsync && !Lua::Function::Get(this->mLuaEnv, "coroutine", "rpc"))
        {
            return XCode::CallLuaFunctionFail;
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
        std::shared_ptr<Message> request;
        if (!this->mConfig->Request.empty())
        {
            request = this->mMsgComponent->New(this->mConfig->Request);
            if (request == nullptr || !message.ParseMessage(request.get()))
            {
                return XCode::CallLuaFunctionFail;
            }
        }
        int count = 1;
        long long userId = 0;
        message.GetHead().Get("id", userId);
        lua_pushinteger(this->mLuaEnv, userId);
        if (request != nullptr)
        {
            count++;
            this->mMsgComponent->Write(this->mLuaEnv, *request);
        }
        if (!this->mConfig->IsAsync)
        {
            return this->Call(count, message);
        }
        return this->CallAsync(count, message);
    }
}
