#include "ServiceMgrComponent.h"
#include <Service/LuaServiceComponent.h>
#include <Service/LocalServiceComponent.h>
#include <Coroutine/CoroutineComponent.h>
#include <Scene/ActionComponent.h>
#include <Util/StringHelper.h>
#include <Core/App.h>
#include <Util/JsonHelper.h>
#include <Scene/ProtocolComponent.h>
#include <Pool/MessagePool.h>
namespace Sentry
{
    bool ServiceMgrComponent::Awake()
    {
		ServerConfig & ServerCfg = App::Get().GetConfig();
		this->mCorComponent = App::Get().GetCoroutineComponent();
        SayNoAssertRetFalse_F(ServerCfg.GetValue("NodeId", this->mNodeId));

        SayNoAssertRetFalse_F(this->mCorComponent = Scene::GetComponent<CoroutineComponent>());
        SayNoAssertRetFalse_F(this->mNetProxyManager = Scene::GetComponent<NetProxyComponent>());
        return true;
    }

	bool ServiceMgrComponent::HandlerMessage(PacketMapper *messageData)
	{
		const std::string & service = messageData->GetService();
		ServiceComponent * localService = this->gameObject->GetComponent<ServiceComponent>(service);
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
		if (method->IsLuaMethod())
		{
			method->Invoke(messageData);
			return true;
		}
		this->mCorComponent->StartCoroutine(&ServiceMgrComponent::Invoke, this, method, messageData);
		return true;
	}
	
	std::string ServiceMgrComponent::GetJson(PacketMapper * messageData)
	{
		RapidJsonWriter jsonWriter;
		jsonWriter.AddParameter("rpcid", messageData->GetRpcId());
		jsonWriter.AddParameter("userid", messageData->GetUserId());
		jsonWriter.AddParameter("method", messageData->GetProConfig()->ServiceName
			+ "." + messageData->GetProConfig()->MethodName);
		const std::string & name = messageData->GetMessageType() < REQUEST_END
			? messageData->GetProConfig()->RequestMsgName : messageData->GetProConfig()->ResponseMsgName;
	
		const std::string & data = messageData->GetMsgBody();
		if (!name.empty() && !data.empty())
		{
			std::string json;		
			Message * message = MessagePool::NewByData(name, data);
			if (message != nullptr && util::MessageToJsonString(*message, &json).ok())
			{
				std::string retData =
					jsonWriter.Serialization() + " \"message\":" + json;
				return retData;
			}
		}
		return jsonWriter.Serialization();
	}

	void ServiceMgrComponent::Invoke(ServiceMethod * method, PacketMapper *messageData)
	{
#ifdef _DEBUG	
		SayNoDebugInfo("[ request ]" << this->GetJson(messageData));
#endif
		XCode code = method->Invoke(messageData);
		const std::string & address = messageData->GetAddress();
		if (!address.empty() && messageData->SetCode(code))
		{
#ifdef _DEBUG	
		SayNoDebugInfo("[ response ]" << this->GetJson(messageData));
#endif
			this->mNetProxyManager->SendNetMessage(messageData);
		}
	}
}// namespace Sentry
