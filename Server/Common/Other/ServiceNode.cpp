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
			TcpProxySession* tcpSession = this->mNetWorkManager->GetProxySession(this->mAddress);
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
		this->PushMessageData(service, method);
		return XCode::Successful;
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
		this->PushMessageData(service, method, &request);
		return XCode::Successful;
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
		shared_ptr<NetWorkWaitCorAction> rpcCallback = NetWorkWaitCorAction::Create(this->mCorManager);
		if (rpcCallback != nullptr)
		{
			this->PushMessageData(service, method, nullptr, rpcCallback);
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
		shared_ptr<NetWorkWaitCorAction> rpcCallback = NetWorkWaitCorAction::Create(this->mCorManager);
		if (rpcCallback != nullptr)
		{
			this->PushMessageData(service, method, nullptr, rpcCallback);
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
		shared_ptr<NetWorkWaitCorAction> rpcCallback = NetWorkWaitCorAction::Create(this->mCorManager);
		if (rpcCallback != nullptr)
		{
			this->PushMessageData(service, method, &request, rpcCallback);
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
		shared_ptr<NetWorkWaitCorAction> rpcCallback = NetWorkWaitCorAction::Create(this->mCorManager);
		if (rpcCallback != nullptr)
		{
			this->PushMessageData(service, method, &request, rpcCallback);
			this->mCorManager->YieldReturn();
			if (response.ParseFromString(rpcCallback->GetMsgData()))
			{
				return rpcCallback->GetCode();
			}
			return XCode::SerializationFailure;
		}
		return XCode::Failure;
	}

	void ServiceNode::PushMessageData(NetMessageProxy *messageData)
	{
		TcpProxySession * tcpSession = this->mNetWorkManager->GetProxySession(this->mAddress);
		if (tcpSession == nullptr)
		{
			this->mMessageQueue.push(messageData);
			this->mNetWorkManager->ConnectByAddress(this->mAddress, this->mNodeName);
		}
		else
		{
			while (!this->mMessageQueue.empty())
			{
				tcpSession->SendMessageData(this->mMessageQueue.front());
				this->mMessageQueue.pop();
			}
			tcpSession->SendMessageData(messageData);
		}
	}

	void ServiceNode::PushMessageData(const std::string &service, const std::string &method, const Message *request, shared_ptr<LocalRetActionProxy> rpcReply)
	{
		NetMessageProxy *msgData = GnetPacketPool.Create();
		if (msgData != nullptr)
		{
			if (request != nullptr)
			{
				std::string messagdData;
				if (request->SerializePartialToString(&messagdData))
				{
					msgData->set_messagedata(messagdData);
					msgData->set_protocname(request->GetTypeName());
				}
			}
			msgData->set_method(method);
			msgData->set_service(service);
			if (rpcReply != nullptr)
			{
#ifdef SOEASY_DEBUG
				rpcReply->mMethod = method;
				rpcReply->mService = service;
#endif
				msgData->set_rpcid(this->mActionManager->AddCallback(rpcReply));
			}
			this->PushMessageData(msgData);
		}
	}
}