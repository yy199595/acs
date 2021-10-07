#include "ServiceMgrComponent.h"
#include <Service/LocalServiceComponent.h>
#include <Coroutine/CoroutineComponent.h>
#include <Scene/ActionComponent.h>
#include <Util/StringHelper.h>
#include <Core/App.h>
#include <Util/JsonHelper.h>
#include <Scene/ProtocolComponent.h>
#include <Pool/MessagePool.h>
#include <NetWork/LuaServiceMethod.h>
namespace Sentry
{
    bool ServiceMgrComponent::Awake()
    {
		ServerConfig & ServerCfg = App::Get().GetConfig();
		this->mCorComponent = App::Get().GetCoroutineComponent();
        SayNoAssertRetFalse_F(ServerCfg.GetValue("NodeId", this->mNodeId));

        SayNoAssertRetFalse_F(this->mCorComponent = this->GetComponent<CoroutineComponent>());
        SayNoAssertRetFalse_F(this->mNetProxyManager = this->GetComponent<NetProxyComponent>());
        return true;
    }

	bool ServiceMgrComponent::HandlerMessage(PacketMapper *messageData)
	{
		const std::string & service = messageData->GetService();
		const ProtocolConfig * config = messageData->GetProConfig();
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
		if (!config->Async) //同步调用
		{
			XCode code = method->Invoke(messageData);
			const std::string & address = messageData->GetAddress();
			if (!address.empty() && messageData->SetCode(code))
			{
				this->mNetProxyManager->SendNetMessage(messageData);
			}
			return true;
		}

		if (method->IsLuaMethod()) // 异步调用
		{
			LuaServiceMethod *  luaMethod = static_cast<LuaServiceMethod*>(method);
			if (luaMethod != nullptr)
			{
				XCode code = luaMethod->AsyncInvoke(messageData);
				if (code != XCode::LuaCoroutineWait)
				{
					SayNoDebugError("call lua function " << service << "." << method << " failure");
					return false;
				}
			}
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
		jsonWriter.AddParameter("method", messageData->GetProConfig()->Service
			+ "." + messageData->GetProConfig()->Method);
		const std::string & name = messageData->GetMessageType() < REQUEST_END
                                   ? messageData->GetProConfig()->Request : messageData->GetProConfig()->Response;
	
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
