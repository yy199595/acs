#include"ServiceNode.h"
#include<Util/StringHelper.h>
#include<Manager/NetWorkManager.h>
#include<Manager/ActionManager.h>
#include<Manager/ServiceManager.h>
#include<NetWork/NetWorkRetAction.h>
#include<Coroutine/CoroutineManager.h>
#include<Util/JsonHelper.h>
namespace SoEasy
{
	ServiceNode::ServiceNode(int areaId, int nodeId, const std::string name, const std::string address, const std::string nAddress)
		: mAreaId(areaId), mNodeId(nodeId), mAddress(address), mNodeName(name), mNoticeAddress(nAddress)
	{
		Applocation * app = Applocation::Get();
		SayNoAssertRet_F(this->mCorManager = app->GetManager<CoroutineManager>());
		SayNoAssertRet_F(this->mActionManager = app->GetManager<ActionManager>());
		SayNoAssertRet_F(this->mNetWorkManager = app->GetManager<NetWorkManager>());
		SayNoAssertRet_F(this->mServiceManager = app->GetManager<ServiceManager>());

		this->mNodeTcpSession = this->mNetWorkManager->GetTcpSession(this->mNoticeAddress);
		if (this->mNodeTcpSession == nullptr)
		{
			std::string ip;
			unsigned short port;
			StringHelper::ParseIpAddress(address, ip, port);
			this->mNodeTcpSession = make_shared<TcpClientSession>(this->mServiceManager, name, ip, port);
		}
	}

	bool ServiceNode::AddService(const std::string & service)
	{
		auto iter = this->mServiceArray.find(service);
		if (iter == this->mServiceArray.end())
		{
			this->mServiceArray.insert(service);
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
		while (!this->mMessageQueue.empty())
		{
			if (!this->mNodeTcpSession->IsActive())
			{
				break;
			}
			this->PushMessageData(this->mMessageQueue.front());
			this->mMessageQueue.pop();
		}
	}

	std::string ServiceNode::GetJsonString()
	{
		RapidJsonWriter jsonWrite;
		jsonWrite.AddParameter("AreaId", this->mAreaId);
		jsonWrite.AddParameter("NodeId", this->mNodeId);
		jsonWrite.AddParameter("Name", this->mNodeName);
		jsonWrite.AddParameter("Address", this->mAddress);
		jsonWrite.AddParameter("Service", this->mServiceArray);
		return jsonWrite.Serialization();
	}

	XCode ServiceNode::Notice(const std::string & service, const std::string & method, const Message * request)
	{
		if (service.empty() || method.empty())
		{
			return XCode::CallArgsError;
		}
		if (this->HasService(service) == false)
		{
			return XCode::CallServiceNotFound;
		}
		this->PushMessageData(service, method, request);
		return XCode::Successful;
	}

	XCode ServiceNode::Call(const std::string & service, const std::string & method)
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

	XCode ServiceNode::Call(const std::string & service, const std::string & method, const Message * request)
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
			this->PushMessageData(service, method, request, rpcCallback);
			this->mCorManager->YieldReturn();
			return rpcCallback->GetCode();
		}
		return XCode::Failure;
	}

	XCode ServiceNode::Call(const std::string & service, const std::string & method, const Message * request, Message & response)
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
			this->PushMessageData(service, method, request, rpcCallback);
			this->mCorManager->YieldReturn();
			if (response.ParseFromString(rpcCallback->GetMsgData()))
			{
				return rpcCallback->GetCode();
			}
			return XCode::SerializationFailure;
		}
		return XCode::Failure;
	}

	void ServiceNode::PushMessageData(SharedPacket messageData)
	{
		if (!this->mNodeTcpSession->IsActive())
		{
			this->mNodeTcpSession->StartConnect();
			this->mMessageQueue.push(messageData);
		}
		else
		{
			const std::string & address = this->mAddress.empty() ? mNoticeAddress : mAddress;
			this->mNetWorkManager->SendMessageByAdress(address, messageData);
		}
	}
	void ServiceNode::PushMessageData(const std::string & service, const std::string & method, const Message * request, shared_ptr<LocalRetActionProxy> rpcReply)
	{
		shared_ptr<PB::NetWorkPacket> msgData = make_shared<PB::NetWorkPacket>();
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