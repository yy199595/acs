#include "NetWorkRetAction.h"
#include <Coroutine/CoroutineComponent.h>
#include <Util/TimeHelper.h>
#include <Core/App.h>
namespace Sentry
{

    LocalLuaRetActionProxy::LocalLuaRetActionProxy(NetLuaRetAction *action) : mBindLuaAction(action)
    {
    }

    void LocalLuaRetActionProxy::Invoke(PacketMapper *backData)
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

    void LocalRetActionProxy1::Invoke(PacketMapper *backData)
    {
    }

    LocalWaitRetActionProxy::LocalWaitRetActionProxy(NetLuaWaitAction *action) : mBindLuaAction(action)
    {
    }

    void LocalWaitRetActionProxy::Invoke(PacketMapper *backData)
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

    NetWorkWaitCorAction::NetWorkWaitCorAction(CoroutineComponent *mgr)
    {
        this->mScheduler = mgr;
        this->mCoroutineId = mgr->GetCurrentCorId();
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

    void NetWorkWaitCorAction::Invoke(PacketMapper *backData)
    {
        this->mCode = backData->GetCode();
        // TODO优化
        this->mMessage = backData->GetMsgBody();
        this->mScheduler->Resume(mCoroutineId);
    }
}// namespace Sentry
