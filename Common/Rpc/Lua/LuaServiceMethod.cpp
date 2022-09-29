#include"LuaServiceMethod.h"
#include"Lua/Function.h"
#include"App/App.h"
#include"Config/ServiceConfig.h"
#include"Lua/LuaServiceTaskSource.h"
#include"Component/ProtoComponent.h"
#include"google/protobuf/util/json_util.h"
namespace Sentry
{

	LuaServiceMethod::LuaServiceMethod(const RpcInterfaceConfig * config, lua_State* lua)
		: ServiceMethod(config->Method), mConfig(config), mLuaEnv(lua)
	{
		this->mMsgComponent = App::Get()->GetComponent<ProtoComponent>();
	}

	XCode LuaServiceMethod::Call(int count, Rpc::Data & message)
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

	XCode LuaServiceMethod::CallAsync(int count, Rpc::Data & message)
    {
        LuaServiceTaskSource *luaTaskSource = new LuaServiceTaskSource(this->mLuaEnv);
        Lua::UserDataParameter::Write(this->mLuaEnv, luaTaskSource);
        if (lua_pcall(this->mLuaEnv, count + 2, 1, 0) != 0)
        {
            delete luaTaskSource;
            message.GetHead().Add("error", lua_tostring(this->mLuaEnv, -1));
            return XCode::CallLuaFunctionFail;
        }
        XCode code = luaTaskSource->Await();

        delete luaTaskSource;
        if (code != XCode::Successful)
        {
            return code;
        }
        if (!this->mConfig->Response.empty() && luaTaskSource->GetRef())
        {
            std::shared_ptr<Message> response = this->mMsgComponent->Read(
                this->mLuaEnv, this->mConfig->Response, -1);
            if (response == nullptr)
            {
                return XCode::JsonCastProtoFailure;
            }
            if (message.WriteMessage(response.get()))
            {
                return XCode::SerializationFailure;
            }
        }
        return XCode::Successful;
    }

	XCode LuaServiceMethod::Invoke(Rpc::Data & message)
    {
        if (this->mConfig->IsAsync && !Lua::Function::Get(this->mLuaEnv, "RpcCall"))
        {
            return XCode::CallLuaFunctionFail;
        }
        const char *tab = this->mConfig->Service.c_str();
        const char *method = this->mConfig->Method.c_str();
        if (!Lua::Function::Get(this->mLuaEnv, tab, method))
        {
            return XCode::NotFoundRpcConfig;
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
