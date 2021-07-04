#include"ServiceNode.h"
#include<Util/StringHelper.h>
#include<Manager/NetProxyManager.h>
#include<Manager/ActionManager.h>
#include<Manager/ServiceManager.h>
#include<NetWork/NetWorkRetAction.h>
#include<Coroutine/CoroutineManager.h>
#include<Util/JsonHelper.h>
#include<Pool/ObjectPool.h>
namespace SoEasy
{
	ServiceNode::ServiceNode(int areaId, int nodeId, const std::string name, const std::string address, const std::string nAddress)
		: mAddress(address), mNodeName(name), mNoticeAddress(nAddress)
	{
		Applocation * app = Applocation::Get();
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

	bool ServiceNode::AddService(const std::string & service)
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

	bool ServiceNode::HasService(const std::string & service)
	{
		auto iter = this->mServiceArray.find(service);
		return iter != this->mServiceArray.end();
	}

	void ServiceNode::OnSystemUpdate()
	{
		if (!this->mMessageQueue.empty())
		{
			while (this->mMessageQueue.empty())
			{
				PB::NetWorkPacket * msgData = this->mMessageQueue.front();
				const std::string & address = mNoticeAddress.empty() ? mAddress : mNoticeAddress;
				if (!this->mNetWorkManager->SendMsgByAddress(address, msgData))
				{
					return;
				}
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

	XCode ServiceNode::Notice(const std::string & service, const std::string & method)
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

	XCode ServiceNode::Notice(const std::string & service, const std::string & method, const Message & request)
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

	XCode ServiceNode::Invoke(const std::string & service, const std::string & method)
	{
		if (service.empty() || method.empty())
		{
			return XCode::CallArgsError;
		}
		if (this->HasService(service) == false)
		{
			return XCode::CallServiceNotFound;
		}
		shared_ptr<NetWorkWaitCorAction> rpcCallback = NetWorkWaitCorAction::Create(method, this->mCorManager);
		if (rpcCallback != nullptr)
		{
			this->PushMessageData(service, method, nullptr, rpcCallback);
			this->mCorManager->YieldReturn();
			return rpcCallback->GetCode();
		}
		return XCode::Failure;
	}

	XCode ServiceNode::Call(const std::string & service, const std::string & method, Message & response)
	{
		if (service.empty() || method.empty())
		{
			return XCode::CallArgsError;
		}
		if (this->HasService(service) == false)
		{
			return XCode::CallServiceNotFound;
		}
		shared_ptr<NetWorkWaitCorAction> rpcCallback = NetWorkWaitCorAction::Create(method, this->mCorManager);
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

	XCode ServiceNode::Invoke(const std::string & service, const std::string & method, const Message & request)
	{
		if (service.empty() || method.empty())
		{
			return XCode::CallArgsError;
		}
		if (this->HasService(service) == false)
		{
			return XCode::CallServiceNotFound;
		}
		shared_ptr<NetWorkWaitCorAction> rpcCallback = NetWorkWaitCorAction::Create(method, this->mCorManager);
		if (rpcCallback != nullptr)
		{
			this->PushMessageData(service, method, &request, rpcCallback);
			this->mCorManager->YieldReturn();
			return rpcCallback->GetCode();
		}
		return XCode::Failure;
	}

	XCode ServiceNode::Call(const std::string & service, const std::string & method, const Message & request, Message & response)
	{
		if (service.empty() || method.empty())
		{
			return XCode::CallArgsError;
		}
		if (this->HasService(service) == false)
		{
			return XCode::CallServiceNotFound;
		}
		shared_ptr<NetWorkWaitCorAction> rpcCallback = NetWorkWaitCorAction::Create(method, this->mCorManager);
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

	void ServiceNode::PushMessageData(PB::NetWorkPacket * messageData)
	{
		const std::string & address = mNoticeAddress.empty() ? mAddress : mNoticeAddress;
		if (!this->mNetWorkManager->SendMsgByAddress(address, messageData))
		{
			this->mMessageQueue.push(messageData);
			this->mNetWorkManager->ConnectByAddress(address, this->mNodeName);
		}
	}

	void ServiceNode::PushMessageData(const std::string & service, const std::string & method, const Message * request, shared_ptr<LocalRetActionProxy> rpcReply)
	{
		PB::NetWorkPacket * msgData = NetPacketPool.Create();
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
			msgData->set_rpcid(this->mActionManager->AddCallback(rpcReply));
			this->PushMessageData(msgData);
		}
	}
}