#include"LuaServiceMethod.h"
#include"Script/LuaInclude.h"
#include"Core/App.h"
#include"Rpc/RpcClientComponent.h"
#include"Scene/LuaScriptComponent.h"
#include"Pool/MessagePool.h"
#include"Scene/RpcConfigComponent.h"
#include"Async/LuaTaskSource.h"
namespace GameKeeper
{

	LuaServiceMethod::LuaServiceMethod(const ProtoConfig * config, lua_State * lua, int idx)
		:ServiceMethod(config->Method), mLuaEnv(lua), mIdx(idx)
	{
        this->mProtoConfig = config;
		this->mScriptComponent = App::Get().GetComponent<LuaScriptComponent>();
		this->mRpcClientComponent = App::Get().GetComponent<RpcClientComponent>();
	}

	XCode LuaServiceMethod::Invoke(const com::Rpc_Request & request, com::Rpc_Response & response)
    {
        int count = 2;
        if (!this->mProtoConfig->IsAsync)
        {
            lua_getfunction(this->mLuaEnv, "Service", "Call");
            lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mIdx);
            if (!lua_isfunction(this->mLuaEnv, -1))
            {
                return XCode::CallLuaFunctionFail;
            }
            lua_pushinteger(this->mLuaEnv, request.userid());
            if (request.has_data() && Helper::Proto::GetJson(request.data(), this->mMessageJson))
            {
                count++;
                const char *json = this->mMessageJson.c_str();
                const size_t size = this->mMessageJson.size();
                lua_pushlstring(this->mLuaEnv, json, size);
            }
            if (lua_pcall(this->mLuaEnv, count, 2, 0) != 0)
            {
                return XCode::CallLuaFunctionFail;
            }
            XCode code = (XCode) lua_tointeger(this->mLuaEnv, -1);
            if (code != XCode::Successful)
            {
                return code;
            }
            size_t size = 0;
            const char *json = lua_tolstring(this->mLuaEnv, -2, &size);
            Message *message = Helper::Proto::NewByJson(this->mProtoConfig->Response, json, size);
            if (message != nullptr)
            {
                response.mutable_data()->PackFrom(*message);
            }
            return XCode::Successful;
        }


        count = 3;
        std::shared_ptr<LuaTaskSource> luaTaskSource(new LuaTaskSource());
        lua_getfunction(this->mLuaEnv, "Service", "CallAsync");
        lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mIdx);
        if (!lua_isfunction(this->mLuaEnv, -1))
        {
            return XCode::CallLuaFunctionFail;
        }
        UserDataParameter::Write(this->mLuaEnv, luaTaskSource);
        lua_pushinteger(this->mLuaEnv, request.userid());
        if (request.has_data() && Helper::Proto::GetJson(request.data(), this->mMessageJson))
        {
            count++;
            const char *json = this->mMessageJson.c_str();
            const size_t size = this->mMessageJson.size();
            lua_pushlstring(this->mLuaEnv, json, size);
        }
        if (lua_pcall(this->mLuaEnv, count, 2, 0) != 0)
        {
            LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
            return XCode::CallLuaFunctionFail;
        }

        XCode code = luaTaskSource->Await();
        if(code != XCode::Successful)
        {
            return code;
        }

        Message * message = Helper::Proto::NewByJson(this->mProtoConfig->Response, luaTaskSource->GetJson());
        if(message != nullptr)
        {
            response.mutable_data()->PackFrom(*message);
        }

        return XCode::Successful;
    }

	int LuaServiceMethod::Response(lua_State * lua)
	{
//		XCode code = (XCode)lua_tointeger(lua, 2);
//        com::Rpc_Request * responseData = (com::Rpc_Request*)lua_touserdata(lua, 1);
//
//		TcpNetProxyComponent * sessionComponent = App::Get().GetComponent<TcpNetProxyComponent>();
//		responseData->ClearMessage();
//		if (lua_isstring(lua, 3))
//		{
//			size_t size = 0;
//			const char * json = lua_tolstring(lua, 3, &size);
//			const std::string & responseName = responseData->GetProConfig()->Response;
//			Message * message = MessagePool::NewByJson(responseName, json, size);
//			if (message != nullptr)
//			{
//				GKAssertBreakFatal_F(message);
//				GKAssertBreakFatal_F(responseData->SetMessage(message));
//			}
//			else
//			{
//				responseData->SetMessage(json, size);
//			}
//		}
//		sessionComponent->SendNetMessage(responseData);

		return 0;
	}
}
