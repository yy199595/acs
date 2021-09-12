#include "ServiceNode.h"
#include <Core/App.h>
#include <Coroutine/CoroutineComponent.h>
#include <Scene/SceneActionComponent.h>
#include <Scene/SceneNetProxyComponent.h>
#include <Service/ServiceMgrComponent.h>
#include <NetWork/NetWorkRetAction.h>
#include <Util/JsonHelper.h>
#include <Util/StringHelper.h>
#include <NetWork/PacketMapper.h>
#include <google/protobuf/util/json_util.h>
namespace Sentry
{
	ServiceNode::ServiceNode(int areaId, int nodeId, const std::string name, const std::string address)
		: mAddress(address), mNodeName(name), mIsClose(false)
	{
		SceneNetProxyComponent * component = Scene::GetComponent<SceneNetProxyComponent>();
		SayNoAssertRet_F(this->mCorComponent = Scene::GetComponent<CoroutineComponent>());
		SayNoAssertRet_F(this->mActionManager = Scene::GetComponent<SceneActionComponent>());
		SayNoAssertRet_F(this->mServiceNodeManager = Service::GetComponent<ServiceNodeComponent>());
		SayNoAssertRet_F(StringHelper::ParseIpAddress(address, this->mIp, this->mPort));

		

		this->mNodeInfoMessage.Clear();
		this->mNodeInfoMessage.set_areaid(areaId);
		this->mNodeInfoMessage.set_nodeid(nodeId);
		this->mNodeInfoMessage.set_servername(name);
		this->mNodeInfoMessage.set_address(address);
		this->mTcpSession = component->Create(address, mNodeName);
		this->mCorId = this->mCorComponent->StartCoroutine(&ServiceNode::HandleMessageSend, this);
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

	void ServiceNode::OnConnectNodeAfter()
	{
		this->mCorComponent->Resume(mCorId);
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
		PacketMapper *messageData = PacketMapper::Create(mAddress, S2S_NOTICE, service, method);
		if (messageData == nullptr)
		{
			SayNoDebugError("not find [" << service << "." << method << "]");
			return XCode::CallArgsError;
		}
		this->AddMessageToQueue(messageData, false);
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
		PacketMapper *messageData = PacketMapper::Create(mAddress, S2S_NOTICE, service, method);
		if (messageData == nullptr)
		{
			SayNoDebugError("not find [" << service << "." << method << "]");
			return XCode::CallArgsError;
		}
		messageData->SetMessage(request);
		this->AddMessageToQueue(messageData, false);
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

		PacketMapper *messageData = PacketMapper::Create(mAddress, S2S_REQUEST, service, method);

		if (messageData == nullptr)
		{
			SayNoDebugError("not find [" << service << "." << method << "]");
			return XCode::CallArgsError;
		}
		return this->SendRpcMessage(messageData);
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

		PacketMapper *messageData = PacketMapper::Create(mAddress, S2S_REQUEST, service, method);

		if (messageData == nullptr)
		{
			SayNoDebugError("not find [" << service << "." << method << "]");
			return XCode::CallArgsError;
		}
		return this->SendRpcMessage(messageData, response);
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

		PacketMapper *messageData = PacketMapper::Create(mAddress, S2S_REQUEST, service, method);

		if (messageData == nullptr)
		{
			SayNoDebugError("not find [" << service << "." << method << "]");
			return XCode::CallArgsError;
		}
		messageData->SetMessage(request);
		return this->SendRpcMessage(messageData);
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

		PacketMapper *messageData = PacketMapper::Create(mAddress, S2S_REQUEST, service, method);

		if (messageData == nullptr)
		{
			SayNoDebugError("not find [" << service << "." << method << "]");
			return XCode::CallArgsError;
		}
		messageData->SetMessage(request);
		return this->SendRpcMessage(messageData, response);
	}

	XCode ServiceNode::SendRpcMessage(PacketMapper * message)
	{
		auto rpcCallback = NetWorkWaitCorAction::Create();
		if (!message->SetRpcId(this->mActionManager->AddCallback(rpcCallback)))
		{
			return XCode::Failure;
		}
		this->AddMessageToQueue(message);
		return rpcCallback->GetCode();
	}

	XCode ServiceNode::SendRpcMessage(PacketMapper * message, Message & response)
	{
		auto rpcCallback = NetWorkWaitCorAction::Create();
		if (!message->SetRpcId(this->mActionManager->AddCallback(rpcCallback)))
		{
			return XCode::Failure;
		}
		this->AddMessageToQueue(message);
		if (rpcCallback->GetCode() == XCode::Successful)
		{
			const std::string & data = rpcCallback->GetMsgData();
			if (response.ParseFromString(data) == false)
			{
				SayNoDebugFatal("parse response message error type : " << response.GetTypeName());
				return XCode::ParseMessageError;
			}
		}
		return rpcCallback->GetCode();
	}

	void ServiceNode::HandleMessageSend()
	{
		while (!this->mIsClose)
		{
			if (this->mNodeMessageQueue.empty())
			{
				this->mCorComponent->YieldReturn();
			}
			while (!this->mTcpSession->IsActive())
			{
				this->mTcpSession->StartConnect();
				this->mCorComponent->YieldReturn();
				if (!this->mTcpSession->IsActive())
				{
					this->mCorComponent->Sleep(3000);
				}
			}

			while (!this->mNodeMessageQueue.empty())
			{
				PacketMapper * message = this->mNodeMessageQueue.front();
				this->mNodeMessageQueue.pop();
				this->mTcpSession->SendMessageData(message);
			}
		}
	}

	void ServiceNode::AddMessageToQueue(PacketMapper * message, bool yield)
	{
		if (message != nullptr)
		{
			this->mNodeMessageQueue.push(message);
			this->mCorComponent->Resume(this->mCorId);
			if (yield == false) return;
			this->mCorComponent->YieldReturn();
		}
	}

}// namespace Sentry