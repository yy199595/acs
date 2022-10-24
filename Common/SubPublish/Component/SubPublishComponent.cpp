//
// Created by zmhy0073 on 2022/10/20.
//

#include"SubPublishComponent.h"
#include"Service/RpcService.h"
#include"Component/InnerNetMessageComponent.h"
#include"Client/Message.h"
namespace Sentry
{
	bool SubPublishComponent::LateAwake()
	{
		const ServerConfig * config = ServerConfig::Inst();
		LOG_CHECK_RET_FALSE(config->GetMember("server", "forward", mLocations));
		LOG_CHECK_RET_FALSE(this->mInnerComponent = this->GetComponent<InnerNetMessageComponent>());
		return true;
	}

	void SubPublishComponent::OnLocalComplete()
	{
		std::vector<RpcService *> rpcServices;
		std::unordered_set<std::string> channels;
		this->mApp->GetComponents<RpcService>(rpcServices);
		std::shared_ptr<Rpc::Packet> message = std::make_shared<Rpc::Packet>();
		{
			message->SetType(Tcp::Type::Request);
			message->SetProto(Tcp::Porto::String);
			message->GetHead().Add("func", "Subscribe");
		}
		for(RpcService * rpcService : rpcServices)
		{
			rpcService->GetSubEventIds(channels);
			for(const std::string & channel : channels)
			{
				auto iter = this->mChannels.find(channel);
				if(iter == this->mChannels.end())
				{
					std::unordered_set<std::string> services;
					this->mChannels.emplace(channel, services);
				}
				this->mChannels[channel].insert(rpcService->GetName());
			}
		}
		this->Subscribe(channels);
		this->Publish("Test1", "112233");
	}

	bool SubPublishComponent::OnMessage(std::shared_ptr<Rpc::Packet> message)
	{
		std::string channel;
		Rpc::Head & head = message->GetHead();
		LOG_CHECK_RET_FALSE(head.Get("channel", channel));
		auto iter = this->mChannels.find(channel);
		if(iter == this->mChannels.end())
		{
			return false;
		}
		for(const std::string & name : iter->second)
		{
			RpcService * rpcService = this->mApp->GetService(name);
			if(rpcService != nullptr)
			{
				rpcService->Invoke(channel, message->GetBody());
			}
		}
		return true;
	}

	int SubPublishComponent::Subscribe(const std::string &channel)
	{
		std::shared_ptr<Rpc::Packet> message = std::make_shared<Rpc::Packet>();
		{
			message->SetContent(channel);
			message->SetType(Tcp::Type::Request);
			message->SetProto(Tcp::Porto::String);
			message->GetHead().Add("func", "Subscribe");
		}
		int count = 0;
		const std::string &address = this->mLocations[0];
		std::shared_ptr<Rpc::Packet> response = this->mInnerComponent->Call(address, message);
		return (response != nullptr && response->GetHead().Get("count", count)) ? count : 0;
	}

	int SubPublishComponent::Subscribe(std::unordered_set<std::string>& channels)
	{
		std::shared_ptr<Rpc::Packet> message = std::make_shared<Rpc::Packet>();
		{
			message->SetType(Tcp::Type::Request);
			message->SetProto(Tcp::Porto::String);
			message->GetHead().Add("func", "Subscribe");
		}
		int count, index = 0;
		std::stringstream ss;
		for (const std::string& channel : channels)
		{
			index++;
			ss << channel;
			if(channels.size() != index)
			{
				ss << "\n";
			}
		}
		message->SetContent(ss.str());
		const std::string& address = this->mLocations[0];
		std::shared_ptr<Rpc::Packet> response = this->mInnerComponent->Call(address, message);
		return (response != nullptr && response->GetHead().Get("count", count)) ? count : 0;
	}


	int SubPublishComponent::UnSubscribe(const std::string &channel)
	{
		std::shared_ptr<Rpc::Packet> message = std::make_shared<Rpc::Packet>();
		{
			message->SetContent(channel);
			message->SetType(Tcp::Type::Request);
			message->SetProto(Tcp::Porto::String);
			message->GetHead().Add("func", "UnSubscribe");
		}
		int count = 0;
		const std::string &address = this->mLocations[0];
		std::shared_ptr<Rpc::Packet> response = this->mInnerComponent->Call(address, message);
		return (response != nullptr && response->GetHead().Get("count", count)) ? count : 0;
	}

	int SubPublishComponent::Publish(const std::string &channel, const std::string &content)
	{
		std::shared_ptr<Rpc::Packet> message = std::make_shared<Rpc::Packet>();
		{
			message->SetContent(content);
			message->SetType(Tcp::Type::Request);
			message->SetProto(Tcp::Porto::String);
			message->GetHead().Add("func", "Publish");
			message->GetHead().Add("channel", channel);
		}
		int count = 0;
		const std::string &address = this->mLocations[0];
		std::shared_ptr<Rpc::Packet> response = this->mInnerComponent->Call(address, message);
		return (response != nullptr && response->GetHead().Get("count", count)) ? count : 0;
	}

}