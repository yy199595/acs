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

        SayNoAssertRetFalse_F(this->mCorComponent = Scene::GetComponent<CoroutineComponent>());
        SayNoAssertRetFalse_F(this->mNetProxyManager = Scene::GetComponent<SceneNetProxyComponent>());
        return true;
    }

	bool ServiceMgrComponent::HandlerMessage(PacketMapper *messageData)
	{
		const std::string & service = messageData->GetService();
		ServiceBase * localService = this->gameObject->GetComponent<ServiceBase>(service);
		if (localService == nullptr)
		{
			SayNoDebugFatal("call service not exist : [" << service << "]");
			return false;
		}
		const std::string & methodName = messageData->GetMethd();
		ServiceMethod * method = localService->GetMethod(methodName);
		if (service == nullptr)
		{
			return false;
			SayNoDebugFatal("call method not exist : [" << service << "." << methodName << "]");
		}
		this->mCorComponent->StartCoroutine(&ServiceMgrComponent::Invoke, this, method, messageData);
		return true;
	}
	
	void ServiceMgrComponent::Invoke(ServiceMethod * method, PacketMapper *messageData)
	{
		XCode code = method->Invoke(messageData);
		const std::string & address = messageData->GetAddress();
		if (!address.empty() && messageData->SetCode(code))
		{
			this->mNetProxyManager->SendNetMessage(messageData);
		}
	}
}// namespace Sentry
