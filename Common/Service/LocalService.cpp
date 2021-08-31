#include "LocalService.h"

#include <Core/App.h>
#include <Scene/SceneActionComponent.h>
#include <Coroutine/CoroutineComponent.h>
#include <Scene/SceneProtocolComponent.h>
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

	void LocalService::GetMethods(std::vector<LocalActionProxy*> & methods)
	{
		auto iter = this->mActionMap.begin();
		for (; iter != this->mActionMap.end(); iter++)
		{
			methods.push_back(iter->second);
		}
	}

	bool LocalService::Awake()
    {
		this->mCorComponent = App::Get().GetCoroutineComponent();     
		return true;
    }

    XCode LocalService::InvokeMethod(PacketMapper *messageData)
    {
        const std::string &method = messageData->GetMethd();
        auto iter = this->mActionMap.find(method);
        if (iter == this->mActionMap.end())
        {
            SayNoDebugError("call <<" << messageData->GetService()
                                      << "." << messageData->GetMethd() << ">> not found");
            return XCode::CallFunctionNotExist;
        }
        LocalActionProxy * localAction = iter->second;
        return localAction->Invoke(messageData);
    }

    bool LocalService::BindFunction(std::string name, LocalAction1 action)
    {
        return this->BindFunction(new LocalActionProxy1(name, action));
    }

    bool LocalService::BindFunction(LocalActionProxy * actionBox)
    {
		SceneProtocolComponent * procolComponent = Scene::GetComponent<SceneProtocolComponent>();
		const std::string & name = actionBox->GetName();
		if (procolComponent->GetProtocolConfig(this->GetServiceName(), name) == nullptr)
		{
			SayNoDebugFatal(this->GetServiceName() << "." << name << " not config");
			return false;
		}
		
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