#include "LocalService.h"

#include <Core/App.h>
#include <Scene/SceneActionComponent.h>
#include <Coroutine/CoroutineComponent.h>

namespace Sentry
{
    LocalService::LocalService()
    {
    }

    bool LocalService::HasMethod(const std::string &action)
    {
        auto iter = this->mActionMap.find(action);
        return iter != this->mActionMap.end();
    }

    bool LocalService::Awake()
    {
		this->mCorComponent = App::Get().GetCoroutineComponent();     
        return ServiceBase::Awake();
    }

    XCode LocalService::InvokeMethod(NetMessageProxy *messageData)
    {
        const std::string &method = messageData->GetMethd();
        auto iter = this->mActionMap.find(method);
        if (iter == this->mActionMap.end())
        {
            SayNoDebugError("call <<" << messageData->GetService()
                                      << "." << messageData->GetMethd() << ">> not found");
            return XCode::CallFunctionNotExist;
        }
        shared_ptr<LocalActionProxy> localAction = iter->second;
        return localAction->Invoke(messageData);
    }

    bool LocalService::BindFunction(std::string name, LocalAction1 action)
    {
        return this->BindFunction(name, make_shared<LocalActionProxy1>(action));
    }

    bool LocalService::BindFunction(const std::string &name, shared_ptr<LocalActionProxy> actionBox)
    {
        auto iter = this->mActionMap.find(name);
        if (iter != this->mActionMap.end())
        {
            SayNoDebugError("register " << this->GetTypeName() << "." << name << " fail");
            return false;
        }
        this->mActionMap.emplace(name, actionBox);
        return true;
    }
}