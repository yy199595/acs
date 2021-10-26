﻿#include "NetWorkRetAction.h"
#include <Coroutine/CoroutineComponent.h>
#include <Util/TimeHelper.h>
#include <Core/App.h>
#include <Scene/LuaScriptComponent.h>
namespace Sentry
{
	void LocalWaitRetActionProxy::Invoke(const com::DataPacket_Response & response)
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

    NetWorkWaitCorAction::NetWorkWaitCorAction(CoroutineComponent *mgr)
    {
        this->mScheduler = mgr;
        this->mCoroutineId = mgr->GetCurrentCorId();
    }

	NetWorkWaitCorAction::~NetWorkWaitCorAction()
	{

	}

	shared_ptr<NetWorkWaitCorAction> NetWorkWaitCorAction::Create()
    {
		CoroutineComponent * corComponent = App::Get().GetCoroutineComponent();
        if (corComponent->IsInMainCoroutine())
        {
            return nullptr;
        }
        return std::make_shared<NetWorkWaitCorAction>(corComponent);
    }

    void NetWorkWaitCorAction::Invoke(const com::DataPacket_Response & response)
    {
		this->mCode = (XCode)response.code();
        this->mMessage = response.messagedata();
        this->mScheduler->Resume(mCoroutineId);
    }

	LocalRetActionProxy::LocalRetActionProxy()
	{
		this->mCreateTime = TimeHelper::GetMilTimestamp();
	}

}// namespace Sentry