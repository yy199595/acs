#include "CallHandler.h"
#include <Util/TimeHelper.h>
#include <Core/App.h>
#include <Define/CommonTypeDef.h>
#include <Scene/LuaScriptComponent.h>
#include <Scene/RpcProtoComponent.h>
#include <Pool/MessagePool.h>
namespace GameKeeper
{
    CallHandler::CallHandler(int method)
    {
        this->mTimerId = 0;
		this->mMethodId = method;
        this->mCreateTime = TimeHelper::GetMilTimestamp();
    }

	void LuaCallHandler::Invoke(const com::Rpc_Response & response)
    {
        LocalObject<com::Rpc_Response> lock(&response);
        lua_pushinteger(this->mCoroutine, (int)response.code());
        if((XCode)response.code() == XCode::Successful)
        {
            std::string json;
            Message *responseMessage = MessagePool::New(response.data());
            GKAssertRet_F(responseMessage && util::MessageToJsonString(*responseMessage, &json).ok());

            auto scriptCom = App::Get().GetComponent<LuaScriptComponent>();
            lua_getref(this->luaEnv, scriptCom->GetLuaRef("Json", "ToObject"));
            if (lua_isfunction(this->luaEnv, -1))
            {
                lua_pushlstring(this->luaEnv, json.c_str(), json.size());
                if (lua_pcall(this->luaEnv, 1, 1, 0) != 0)
                {
                    GKDebugError(lua_tostring(this->luaEnv, -1));
                    return;
                }
                lua_xmove(this->luaEnv, this->mCoroutine, 1);
                lua_presume(this->mCoroutine, this->luaEnv, 2);
                return;
            }
        }
        lua_presume(this->mCoroutine, this->luaEnv, 1);
    }

    CppCallHandler::CppCallHandler(int method)
		:CallHandler(method)
    {		
        this->mCoroutineId = 0;
		this->mScheduler = App::Get().GetCorComponent();
    }

    void CppCallHandler::Invoke(const com::Rpc_Response & response)
    {
		this->mCode = (XCode)response.code();
        if(this->mMessage != nullptr && this->mCode == XCode::Successful)
        {
			if (!response.data().UnpackTo(this->mMessage))
			{
				this->mCode = XCode::ParseMessageError;
			}
        }
        this->mScheduler->Resume(mCoroutineId);
    }

    XCode CppCallHandler::StartCall()
    {    
        this->mScheduler->YieldReturn(this->mCoroutineId);
        return this->mCode;
    }

    XCode CppCallHandler::StartCall(google::protobuf::Message &message)
    {
        this->mMessage = &message;
        this->mScheduler->YieldReturn(this->mCoroutineId);
        return this->mCode;
    }
}// namespace GameKeeper
