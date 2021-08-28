#include "ServiceMgrComponent.h"
#include <Service/LocalLuaService.h>
#include <Service/LocalService.h>
#include <Coroutine/CoroutineComponent.h>
#include <Scene/SceneActionComponent.h>
#include <Util/StringHelper.h>
#include <Core/App.h>

namespace Sentry
{
    bool ServiceMgrComponent::Awake()
    {
		ServerConfig & ServerCfg = App::Get().GetConfig();
		this->mCorComponent = App::Get().GetCoroutineComponent();
        SayNoAssertRetFalse_F(ServerCfg.GetValue("NodeId", this->mNodeId));
		
        SayNoAssertRetFalse_F(this->mActionManager = Scene::GetComponent<SceneActionComponent>());
        SayNoAssertRetFalse_F(this->mCorComponent = Scene::GetComponent<CoroutineComponent>());
        SayNoAssertRetFalse_F(this->mNetProxyManager = Scene::GetComponent<SceneNetProxyComponent>());
        return true;
    }

    bool ServiceMgrComponent::HandlerMessage(NetMessageProxy *messageData)
    {
		const std::string & service = messageData->GetService();
		ServiceBase * localService = this->gameObject->GetComponent<ServiceBase>(service);
        if (localService == nullptr || localService->HasMethod(messageData->GetMethd()))
        {
            return false;
        }
		this->mCorComponent->StartCoroutine(&ServiceMgrComponent::Invoke1, this, messageData);
        return true;
    }

    bool ServiceMgrComponent::HandlerMessage(const std::string &address, NetMessageProxy *messageData)
    {
        const std::string &method = messageData->GetMethd();
        const std::string &service = messageData->GetService();
		ServiceBase *localService = this->gameObject->GetComponent<ServiceBase>(service);
		if (localService == nullptr || localService->HasMethod(method) == false)
		{
			SayNoDebugError("call function not find [" << service << "." << method << "]");
			return false;
		}             
		this->mCorComponent->StartCoroutine(&ServiceMgrComponent::Invoke2, this, address, messageData);
        return true;
    }
	void ServiceMgrComponent::Invoke1(NetMessageProxy *messageData)
	{

	}

	void ServiceMgrComponent::Invoke2(const std::string & address, NetMessageProxy * messageData)
	{
		const std::string &service = messageData->GetService();
		ServiceBase *localService = this->gameObject->GetComponent<ServiceBase>(service);
		XCode code = localService->InvokeMethod(messageData);
		
		messageData->SetCode(code);
		this->mNetProxyManager->SendMsgByAddress(address, messageData);
	}
}// namespace Sentry
