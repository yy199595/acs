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
		
	}

	void ServiceNode::OnConnectSuccessful()
	{
		while (!this->mConnectCoroutines.empty())
		{
			unsigned int id = this->mConnectCoroutines.front();
			this->mConnectCoroutines.pop();
			this->mCorComponent->Resume(id);
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
		PacketMapper *messageData = PacketMapper::Create(mAddress, S2S_NOTICE, service, method);
		if (messageData == nullptr)
		{
			SayNoDebugError("not find [" << service << "." << method << "]");
			return XCode::CallArgsError;
		}
		return this->SendRpcMessage(messageData);
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
		return this->SendRpcMessage(messageData);
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

	TcpProxySession * ServiceNode::GetNodeSession()
	{
		TcpProxySession *tcpSession = this->mNetWorkManager->GetProxySession(this->mAddress);
		if (tcpSession == nullptr)
		{
			this->mNetWorkManager->ConnectByAddress(this->mAddress, this->mNodeName);
		}
		return this->mNetWorkManager->GetProxySession(this->mAddress);
	}
	XCode ServiceNode::SendRpcMessage(PacketMapper * message)
	{
		auto rpcCallback = NetWorkWaitCorAction::Create(this->mCorComponent);
		unsigned int rpcId = this->mActionManager->AddCallback(rpcCallback);
		TcpProxySession *tcpSession = this->mNetWorkManager->GetProxySession(this->mAddress);
		if (tcpSession == nullptr)
		{
			tcpSession = this->mNetWorkManager->ConnectByAddress(this->mAddress, this->mNodeName);
			if (!tcpSession->IsActive())
			{
				unsigned int id = this->mCorComponent->GetCurrentCorId();
				this->mConnectCoroutines.push(id);
				this->mCorComponent->YieldReturn();
			}
		}

		message->SetRpcId(rpcId);
		if (!tcpSession->SendMessageData(message))
		{
			return XCode::SendMessageFail;
		}
		this->mCorComponent->YieldReturn();
		return rpcCallback->GetCode();
	}
	XCode ServiceNode::SendRpcMessage(PacketMapper * message, Message & response)
	{
		auto rpcCallback = NetWorkWaitCorAction::Create(this->mCorComponent);
		unsigned int rpcId = this->mActionManager->AddCallback(rpcCallback);
		TcpProxySession *tcpSession = this->mNetWorkManager->GetProxySession(this->mAddress);
		if (tcpSession == nullptr)
		{
			tcpSession = this->mNetWorkManager->ConnectByAddress(this->mAddress, this->mNodeName);
			if (!tcpSession->IsActive())
			{
				unsigned int id = this->mCorComponent->GetCurrentCorId();
				this->mConnectCoroutines.push(id);
				this->mCorComponent->YieldReturn();
			}
		}
		
		message->SetRpcId(rpcId);
		if (!tcpSession->SendMessageData(message))
		{
			return XCode::SendMessageFail;
		}
		this->mCorComponent->YieldReturn();
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
}// namespace Sentry