#include"LuaServiceMethod.h"
#include"Script/LuaInclude.h"
#include"Core/App.h"
#include"Rpc/RpcClientComponent.h"
#include"Scene/LuaScriptComponent.h"
#include"Pool/MessagePool.h"
#include"Scene/RpcConfigComponent.h"
#include"Async/LuaTaskSource.h"
namespace Sentry
{

	LuaServiceMethod::LuaServiceMethod(const ProtoConfig * config, lua_State * lua, int idx)
		:ServiceMethod(config->Method), mLuaEnv(lua), mIdx(idx)
	{
        this->mProtoConfig = config;
		this->mScriptComponent = App::Get().GetComponent<LuaScriptComponent>();
		this->mRpcClientComponent = App::Get().GetComponent<RpcClientComponent>();
	}

    tuple<XCode, std::shared_ptr<Message>> LuaServiceMethod::Call(long long id, const std::string &json)
    {
        lua_getfunction(this->mLuaEnv, "Service", "Call");
        lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mIdx);
        if (!lua_isfunction(this->mLuaEnv, -1)) {
            return make_tuple(XCode::CallLuaFunctionFail, nullptr);
        }
        LuaParameter::WriteArgs(this->mLuaEnv, id, json);
        if (lua_pcall(this->mLuaEnv, 3, 2, 0) != 0) {
            LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
            return make_tuple(XCode::CallLuaFunctionFail, nullptr);
        }
        XCode code = (XCode) lua_tointeger(this->mLuaEnv, -1);
        const std::string &name = this->mProtoConfig->Response;
        if (code != XCode::Successful || name.empty()) {
            return make_tuple(code, nullptr);
        }

        size_t size = 0;
        const char *str = lua_tolstring(this->mLuaEnv, -2, &size);
        Message *message = Helper::Proto::NewByJson(name, str, size, true);
        return make_tuple(XCode::Successful, std::shared_ptr<Message>(message));
    }

    tuple<XCode, std::shared_ptr<Message>> LuaServiceMethod::CallAsync(long long id, const std::string &json)
    {
        lua_getfunction(this->mLuaEnv, "Service", "CallAsync");
        lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mIdx);
        if (!lua_isfunction(this->mLuaEnv, -1)) {
            return make_tuple(XCode::CallLuaFunctionFail, nullptr);
        }
        lua_pushinteger(this->mLuaEnv, id);
        lua_pushlstring(this->mLuaEnv, json.c_str(), json.size());
        if (lua_pcall(this->mLuaEnv, 3, 1, 0) != 0) {
            LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
            return make_tuple(XCode::CallLuaFunctionFail, nullptr);
        }
        LuaTaskSource * luaTaskSource = PtrProxy<LuaTaskSource>::Read(this->mLuaEnv, -1);
        if(luaTaskSource == nullptr)
        {
            return make_tuple(XCode::CallLuaFunctionFail, nullptr);
        }
        XCode code = luaTaskSource->Await();
        const std::string &name = this->mProtoConfig->Response;
        if (code != XCode::Successful || name.empty()) {
            return make_tuple(code, nullptr);
        }
        const std::string &response = luaTaskSource->GetJson();
        Message *message = Helper::Proto::NewByJson(name, response, true);
        return make_tuple(XCode::Successful, std::shared_ptr<Message>(message));
    }

	XCode LuaServiceMethod::Invoke(const com::Rpc_Request & request, com::Rpc_Response & response)
    {
        std::string json;
        if(request.has_data() && !Helper::Proto::GetJson(request.data(), json))
        {
            return XCode::ProtoCastJsonFailure;
        }
        auto luaResponse = this->mProtoConfig->IsAsync
                ? this->CallAsync(request.user_id(), json) : this->Call(request.user_id(), json);
        if(std::get<1>(luaResponse) != nullptr)
        {
            auto message = std::get<1>(luaResponse);
            response.mutable_data()->PackFrom(*message);
        }
        return std::get<0>(luaResponse);
    }
}
