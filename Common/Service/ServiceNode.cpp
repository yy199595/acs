#include "ServiceNode.h"
#include <Core/App.h>
#include <Coroutine/CoroutineComponent.h>
#include <Scene/SceneActionComponent.h>
#include <Scene/SceneNetProxyComponent.h>
#include <Service/ServiceMgrComponent.h>
#include <NetWork/NetWorkRetAction.h>
#include <Util/JsonHelper.h>
#include <Util/StringHelper.h>
#include <NetWork/NetMessageProxy.h>
namespace Sentry
{
	ServiceNode::ServiceNode(int areaId, int nodeId, const std::string name, const std::string address)
		: mAddress(address), mNodeName(name)
	{
		
		SayNoAssertRet_F(this->mCorComponent = Scene::GetComponent<CoroutineComponent>());
		SayNoAssertRet_F(this->mActionManager = Scene::GetComponent<SceneActionComponent>());
		SayNoAssertRet_F(this->mNetWorkManager = Scene::GetComponent<SceneNetProxyComponent>());
		SayNoAssertRet_F(this->mServiceNodeManager = Service::GetComponent<ServiceNodeComponent>());
		SayNoAssertRet_F(StringHelper::ParseIpAddress(address, this->mIp, this->mPort));

		this->mNodeInfoMessage.Clear();
		this->mNodeInfoMessage.set_areaid(areaId);
		this->mNodeInfoMessage.set_nodeid(nodeId);
		this->mNodeInfoMessage.set_servername(name);
		this->mNodeInfoMessage.set_address(address);
	}

	bool ServiceNode::AddService(const std::string &service)
	{
		auto iter = this->mServiceArray.find(service);
		if (iter == this->mServiceArray.end())
		{
			this->mServiceArray.insert(service);
			this->mNodeInfoMessage.add_services(service);
			return true;
		}
		return false;
	}

	bool ServiceNode::HasService(const std::string &service)
	{
		auto iter = this->mServiceArray.find(service);
		return iter != this->mServiceArray.end();
	}

	void ServiceNode::OnFrameUpdate(float t)
	{
		if (!this->mMessageQueue.empty())
		{
			TcpProxySession *tcpSession = this->mNetWorkManager->GetProxySession(this->mAddress);
			while (tcpSession != nullptr && !this->mMessageQueue.empty())
			{
				tcpSession->SendMessageData(this->mMessageQueue.front());
				this->mMessageQueue.pop();
			}
		}
	}

	std::string ServiceNode::GetJsonString()
	{
		std::string json;
		util::MessageToJsonString(this->mNodeInfoMessage, &json);
		return json;
	}

	XCode ServiceNode::Notice(const std::string &service, const std::string &method)
	{
		if (service.empty() || method.empty())
		{
			return XCode::CallArgsError;
		}
		if (this->HasService(service) == false)
		{
			return XCode::CallServiceNotFound;
		}
		NetMessageProxy *messageData = NetMessageProxy::Create(mAddress, S2S_NOTICE, service, method);
		if (messageData == nullptr)
		{
			SayNoDebugError("not find [" << service << "." << method << "]");
			return XCode::CallArgsError;
		}
		return this->PushMessage(messageData);
	}

	XCode ServiceNode::Notice(const std::string &service, const std::string &method, const Message &request)
	{
		if (service.empty() || method.empty())
		{
			return XCode::CallArgsError;
		}
		if (this->HasService(service) == false)
		{
			return XCode::CallServiceNotFound;
		}
		NetMessageProxy *messageData = NetMessageProxy::Create(mAddress, S2S_NOTICE, service, method);
		if (messageData == nullptr)
		{
			SayNoDebugError("not find [" << service << "." << method << "]");
			return XCode::CallArgsError;
		}
		messageData->SetMessage(request);
		return this->PushMessage(messageData);
	}

	XCode ServiceNode::Invoke(const std::string &service, const std::string &method)
	{
		if (service.empty() || method.empty())
		{
			return XCode::CallArgsError;
		}

		if (this->HasService(service) == false)
		{
			return XCode::CallServiceNotFound;
		}

		NetMessageProxy *messageData = NetMessageProxy::Create(mAddress, S2S_REQUEST, service, method);

		if (messageData == nullptr)
		{
			SayNoDebugError("not find [" << service << "." << method << "]");
			return XCode::CallArgsError;
		}

		auto rpcCallback = NetWorkWaitCorAction::Create(this->mCorComponent);
		if (this->PushMessage(messageData))
		{

		}
		long long rpcId = this->mActionManager->AddCallback(rpcCallback);

		messageData->SetRpcId(rpcId);
		this->PushMessage(messageData);
		this->mCorComponent->YieldReturn();
		return rpcCallback->GetCode();
	}

	XCode ServiceNode::Call(const std::string &service, const std::string &method, Message &response)
	{
		if (service.empty() || method.empty())
		{
			return XCode::CallArgsError;
		}
		if (this->HasService(service) == false)
		{
			return XCode::CallServiceNotFound;
		}

		NetMessageProxy *messageData = NetMessageProxy::Create(mAddress, S2S_REQUEST, service, method);

		if (messageData == nullptr)
		{
			SayNoDebugError("not find [" << service << "." << method << "]");
			return XCode::CallArgsError;
		}

		auto rpcCallback = NetWorkWaitCorAction::Create(this->mCorComponent);
		unsigned int rpcId = this->mActionManager->AddCallback(rpcCallback);

		if (rpcId > 0)
		{
			messageData->SetRpcId(rpcId);
			this->PushMessage(messageData);
			this->mCorComponent->YieldReturn();
			if (response.ParseFromString(rpcCallback->GetMsgData()))
			{
				return rpcCallback->GetCode();
			}
			return XCode::SerializationFailure;
		}
		return XCode::Failure;
	}

	XCode ServiceNode::Invoke(const std::string &service, const std::string &method, const Message &request)
	{
		if (service.empty() || method.empty())
		{
			return XCode::CallArgsError;
		}
		if (this->HasService(service) == false)
		{
			return XCode::CallServiceNotFound;
		}

		NetMessageProxy *messageData = NetMessageProxy::Create(mAddress, S2S_REQUEST, service, method);

		if (messageData == nullptr)
		{
			SayNoDebugError("not find [" << service << "." << method << "]");
			return XCode::CallArgsError;
		}

		auto rpcCallback = NetWorkWaitCorAction::Create(this->mCorComponent);
		unsigned int rpcId = this->mActionManager->AddCallback(rpcCallback);
		if (rpcId > 0 && messageData->SetMessage(request))
		{
			messageData->SetRpcId(rpcId);
			this->PushMessage(messageData);
			this->mCorComponent->YieldReturn();
			return rpcCallback->GetCode();
		}
		return XCode::Failure;
	}

	XCode ServiceNode::Call(const std::string &service, const std::string &method, const Message &request, Message &response)
	{
		if (service.empty() || method.empty())
		{
			return XCode::CallArgsError;
		}
		if (this->HasService(service) == false)
		{
			return XCode::CallServiceNotFound;
		}

		NetMessageProxy *messageData = NetMessageProxy::Create(mAddress, S2S_REQUEST, service, method);

		if (messageData == nullptr)
		{
			SayNoDebugError("not find [" << service << "." << method << "]");
			return XCode::CallArgsError;
		}

		auto rpcCallback = NetWorkWaitCorAction::Create(this->mCorComponent);
		unsigned int rpcId = this->mActionManager->AddCallback(rpcCallback);
		if (rpcId > 0 && messageData->SetMessage(request))
		{
			messageData->SetRpcId(rpcId);
			this->PushMessage(messageData);
			this->mCorComponent->YieldReturn();
			if (response.ParseFromString(rpcCallback->GetMsgData()))
			{
				return rpcCallback->GetCode();
			}
			return XCode::SerializationFailure;
		}
		return XCode::Failure;
	}

	XCode ServiceNode::PushMessage(NetMessageProxy *messageData)
	{
		TcpProxySession *tcpSession = this->mNetWorkManager->GetProxySession(this->mAddress);
		if (tcpSession == nullptr)
		{
			this->mMessageQueue.push(messageData);
			this->mNetWorkManager->ConnectByAddress(this->mAddress, this->mNodeName);
			// TODO
		}
		else
		{
			while (!this->mMessageQueue.empty())
			{
				tcpSession->SendMessageData(this->mMessageQueue.front());
				this->mMessageQueue.pop();
			}
			if (tcpSession->SendMessageData(messageData))
			{
				return XCode::Successful;
			}
		}
		return XCode::SendMessageFail;
	}
}// namespace Sentry