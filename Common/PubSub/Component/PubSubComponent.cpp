//
// Created by 64658 on 2025/4/10.
//

#include "PubSubComponent.h"
#include "XCode/XCode.h"
#include "Entity/Actor/App.h"
#include "PubSub/Service/PubSubSystem.h"
#include "Node/Component/NodeComponent.h"
#include "Cluster/Config/ClusterConfig.h"

namespace acs
{
	PubSubComponent::PubSubComponent()
	{
		this->mNode = nullptr;
	}

	bool PubSubComponent::LateAwake()
	{
		std::string name(ComponentFactory::GetName<PubSubSystem>());
		if (!ClusterConfig::Inst()->GetServerName(name, this->mNodeName))
		{
			LOG_ERROR("not find node service:{}", name);
			return false;
		}
		this->mNode = this->GetComponent<NodeComponent>();
		return true;
	}

	int PubSubComponent::Publish(const std::string& channel, const std::string& message)
	{
		if(RpcConfig::Inst()->GetMethodConfig(channel) == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		Node * node = this->mNode->Next(this->mNodeName);
		if(node == nullptr)
		{
			return XCode::NotFoundActor;
		}
		std::unique_ptr<rpc::Message> request = node->Make("EventSystem.Publish");
		if(request == nullptr)
		{
			return XCode::MakeTcpRequestFailure;
		}
		request->SetContent(rpc::Proto::Json, message);
		request->GetHead().Add(rpc::Header::channel, channel);
		return node->Call(std::move(request));
	}

	int PubSubComponent::Publish(const std::string& channel, const json::w::Document& document)
	{
		std::string message;
		if(!document.Encode(&message))
		{
			return XCode::ParseJsonFailure;
		}
		return this->Publish(channel, message);
	}

	int PubSubComponent::Subscribe(const std::string& channel)
	{
		if(RpcConfig::Inst()->GetMethodConfig(channel) == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		Node * node = this->mNode->Next(this->mNodeName);
		if(node == nullptr)
		{
			return XCode::NotFoundActor;
		}
		std::unique_ptr<rpc::Message> request = node->Make("EventSystem.Subscribe");
		if(request == nullptr)
		{
			return XCode::MakeTcpRequestFailure;
		}
		request->GetHead().Add(rpc::Header::channel, channel);
		return node->Call(std::move(request));
	}

	int PubSubComponent::UnSubscribe(const std::string& channel)
	{
		if(RpcConfig::Inst()->GetMethodConfig(channel) == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		Node * node = this->mNode->Next(this->mNodeName);
		if(node == nullptr)
		{
			return XCode::NotFoundActor;
		}
		std::unique_ptr<rpc::Message> request = node->Make("EventSystem.UnSubscribe");
		if(request != nullptr)
		{
			return XCode::MakeTcpRequestFailure;
		}
		request->GetHead().Add(rpc::Header::channel, channel);
		return node->Call(std::move(request));
	}
}