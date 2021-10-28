#include "CallHandler.h"
#include <Coroutine/CoroutineComponent.h>
#include <Util/TimeHelper.h>
#include <Core/App.h>
#include <Scene/LuaScriptComponent.h>
namespace Sentry
{
    CallHandler::CallHandler()
    {
        this->mCreateTime = TimeHelper::GetMilTimestamp();
    }

	void LuaCallHandler::Invoke(const com::DataPacket_Response & response)
    {
//		auto config = backData->GetProConfig();
//		lua_pushinteger(this->mCoroutine, (int)backData->GetCode());
//
//		if (!backData->GetMsgBody().empty())
//		{
//			const char * json = backData->GetMsgBody().c_str();
//			const size_t size = backData->GetMsgBody().size();
//			auto scriptCom = App::Get().GetComponent<LuaScriptComponent>();
//			lua_getref(this->luaEnv, scriptCom->GetLuaRef("Json", "ToObject"));
//			if (lua_isfunction(this->luaEnv, -1))
//			{
//				lua_pushlstring(this->luaEnv, json, size);
//				if (lua_pcall(this->luaEnv, 1, 1, 0) != 0)
//				{
//					SayNoDebugError(lua_tostring(this->luaEnv, -1));
//					return;
//				}
//				lua_xmove(this->luaEnv, this->mCoroutine, 1);
//				lua_presume(this->mCoroutine, this->luaEnv, 2);
//				return;
//			}
//		}
//		lua_presume(this->mCoroutine, this->luaEnv, 1);
    }

    CppCallHandler::CppCallHandler()
    {
        this->mCoroutineId = 0;
        this->mScheduler = nullptr;

    }

	CppCallHandler::~CppCallHandler()
	{

	}

    void CppCallHandler::Invoke(const com::DataPacket_Response & response)
    {
		this->mCode = (XCode)response.code();
        if(this->mMessage != nullptr)
        {
            if(!this->mMessage->ParseFromString(response.messagedata()))
            {
                this->mCode = XCode::ParseMessageError;
            }
        }
        this->mScheduler->Resume(mCoroutineId);
    }

    XCode CppCallHandler::StartCall()
    {
        this->mScheduler = App::Get().GetCorComponent();
        this->mScheduler->YieldReturn(this->mCoroutineId);
        return this->mCode;
    }

    XCode CppCallHandler::StartCall(google::protobuf::Message &message)
    {
        this->mMessage = &message;
        this->mScheduler = App::Get().GetCorComponent();
        this->mScheduler->YieldReturn(this->mCoroutineId);
        return this->mCode;
    }



}// namespace Sentry
