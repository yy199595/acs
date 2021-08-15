#include "NetWorkRetAction.h"
#include <Coroutine/CoroutineManager.h>

namespace Sentry
{

    LocalLuaRetActionProxy::LocalLuaRetActionProxy(NetLuaRetAction *action) : mBindLuaAction(action)
    {
    }

    void LocalLuaRetActionProxy::Invoke(NetMessageProxy *backData)
    {
        XCode code = backData->GetCode();
        //TODO
        /*Message * pMessage = backData->GetResMessage();
        if (pMessage != nullptr)
        {
            this->mBindLuaAction->Inovke(code, pMessage);
            return;
        }
        this->mBindLuaAction->Inovke(code);*/
    }

    LocalRetActionProxy::LocalRetActionProxy()
    {
        this->mCreateTime = TimeHelper::GetMilTimestamp();
    }

    void LocalRetActionProxy1::Invoke(NetMessageProxy *backData)
    {
    }

    LocalWaitRetActionProxy::LocalWaitRetActionProxy(NetLuaWaitAction *action) : mBindLuaAction(action)
    {
    }

    void LocalWaitRetActionProxy::Invoke(NetMessageProxy *backData)
    {
        XCode code = backData->GetCode();
        // TODO
        //Message * pMessage = backData->GetResMessage();
        //if (pMessage != nullptr)
        //{
        //	this->mBindLuaAction->Inovke(code, pMessage);
        //	return;
        //}
        //const std::string & json = backData->GetJsonData();
        //if (!json.empty())
        //{
        //	this->mBindLuaAction->Inovke(code, json);
        //	return;
        //}
        //this->mBindLuaAction->Inovke(code);
    }

    NetWorkWaitCorAction::NetWorkWaitCorAction(CoroutineManager *mgr)
    {
        this->mScheduler = mgr;
        this->mCoroutineId = mgr->GetCurrentCorId();
    }

    shared_ptr<NetWorkWaitCorAction> NetWorkWaitCorAction::Create(CoroutineManager *coroutineMgr)
    {
        if (coroutineMgr->IsInMainCoroutine())
        {
            return nullptr;
        }
        return std::make_shared<NetWorkWaitCorAction>(coroutineMgr);
    }

    void NetWorkWaitCorAction::Invoke(NetMessageProxy *backData)
    {
        this->mCode = backData->GetCode();
        // TODO优化
        this->mMessage = backData->GetMsgBody();
        this->mScheduler->Resume(mCoroutineId);
    }
}// namespace Sentry
