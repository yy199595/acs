#include "ServiceNode.h"
#include <Util/StringHelper.h>
#include <Manager/NetProxyManager.h>
#include <Manager/ActionManager.h>
#include <Manager/ServiceManager.h>
#include <NetWork/NetWorkRetAction.h>
#include <Coroutine/CoroutineManager.h>
#include <Util/JsonHelper.h>
#include <Pool/ObjectPool.h>
namespace Sentry
{
	ServiceNode::ServiceNode(int areaId, int nodeId, const std::string name, const std::string address)
		: mAddress(address), mNodeName(name)
	{
		Applocation *app = Applocation::Get();
		SayNoAssertRet_F(this->mCorManager = app->GetManager<CoroutineManager>());
		SayNoAssertRet_F(this->mActionManager = app->GetManager<ActionManager>());
		SayNoAssertRet_F(this->mNetWorkManager = app->GetManager<NetProxyManager>());
		SayNoAssertRet_F(this->mServiceNodeManager = app->GetManager<ServiceNodeManager>());
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
		NetMessageProxy *messageData = NetMessageProxy::Create(s2sNotice, service, method);
		if (messageData == nullptr)
		{
			SayNoDebugError("not find [" << service << "." << method << "]");
			return XCode::CallArgsError;
		}
		return this->PushMessageData(messageData);
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
		NetMessageProxy *messageData = NetMessageProxy::Create(s2sNotice, service, method);
		if (messageData == nullptr)
		{
			SayNoDebugError("not find [" << service << "." << method << "]");
			return XCode::CallArgsError;
		}
		messageData->InitMessageParame(request.New());
		return this->PushMessageData(messageData);
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

		NetMessageProxy *messageData = NetMessageProxy::Create(s2sRequest, service, method);

		if (messageData == nullptr)
		{
			SayNoDebugError("not find [" << service << "." << method << "]");
			return XCode::CallArgsError;
		}

		shared_ptr<NetWorkWaitCorAction> rpcCallback = NetWorkWaitCorAction::Create(this->mCorManager);

		if (messageData->InitMessageParame(nullptr, this->mActionManager->AddCallback(rpcCallback)))
		{
			this->PushMessageData(messageData);
			this->mCorManager->YieldReturn();
			return rpcCallback->GetCode();
		}
		return XCode::Failure;
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

		NetMessageProxy *messageData = NetMessageProxy::Create(s2sRequest, service, method);

		if (messageData == nullptr)
		{
			SayNoDebugError("not find [" << service << "." << method << "]");
			return XCode::CallArgsError;
		}

		shared_ptr<NetWorkWaitCorAction> rpcCallback = NetWorkWaitCorAction::Create(this->mCorManager);
		long long rpcId = this->mActionManager->AddCallback(rpcCallback);

		if (rpcId > 0 && messageData->InitMessageParame(nullptr, rpcId))
		{
			this->PushMessageData(messageData);
			this->mCorManager->YieldReturn();
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

		NetMessageProxy *messageData = NetMessageProxy::Create(s2sRequest, service, method);

		if (messageData == nullptr)
		{
			SayNoDebugError("not find [" << service << "." << method << "]");
			return XCode::CallArgsError;
		}

		shared_ptr<NetWorkWaitCorAction> rpcCallback = NetWorkWaitCorAction::Create(this->mCorManager);
		long long rpcId = this->mActionManager->AddCallback(rpcCallback);
		if (rpcId > 0 && messageData->InitMessageParame(request.New(), rpcId))
		{
			this->PushMessageData(messageData);
			this->mCorManager->YieldReturn();
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

		NetMessageProxy *messageData = NetMessageProxy::Create(s2sRequest, service, method);

		if (messageData == nullptr)
		{
			SayNoDebugError("not find [" << service << "." << method << "]");
			return XCode::CallArgsError;
		}

		shared_ptr<NetWorkWaitCorAction> rpcCallback = NetWorkWaitCorAction::Create(this->mCorManager);
		long long rpcId = this->mActionManager->AddCallback(rpcCallback);
		if (rpcId > 0 && messageData->InitMessageParame(request.New()))
		{
			this->PushMessageData(messageData);
			this->mCorManager->YieldReturn();
			if (response.ParseFromString(rpcCallback->GetMsgData()))
			{
				return rpcCallback->GetCode();
			}
			return XCode::SerializationFailure;
		}
		return XCode::Failure;
	}

	XCode ServiceNode::PushMessageData(NetMessageProxy *messageData)
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
}