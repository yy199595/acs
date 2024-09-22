//
// Created by leyi on 2024/1/18.
//

#include"RedisSubComponent.h"
#include"Entity/Actor/App.h"
#include"Server/Component/ThreadComponent.h"
#include "Lua/Engine/ModuleClass.h"

namespace acs
{
	RedisSubComponent::RedisSubComponent()
	{
		this->mClient = nullptr;
	}

	bool RedisSubComponent::Awake()
	{
		std::unique_ptr<json::r::Value> redisObject;
		if(!ServerConfig::Inst()->Get("sub", redisObject))
		{
			return false;
		}
		this->mConfig.Debug = false;
		redisObject->Get("ping", this->mConfig.Ping);
		redisObject->Get("debug", this->mConfig.Debug);
		redisObject->Get("passwd", this->mConfig.Password);
		redisObject->Get("address", this->mConfig.Address);
		return true;
	}

	bool RedisSubComponent::LateAwake()
	{
		const std::string & address = this->mConfig.Address;
		ThreadComponent* component = this->GetComponent<ThreadComponent>();
		{
			tcp::Socket * sock = component->CreateSocket(address);
			this->mClient = new redis::Client(sock, this->mConfig, this);
			if(!this->mClient->Start())
			{
				LOG_ERROR("start sub redis [{}] fail", address);
				return false;
			}
		}
		if(this->mConfig.Debug)
		{
			std::unique_ptr<redis::Request> request =
					redis::Request::Make("MONITOR");
			this->mClient->Sync(std::move(request));
		}
		return true;
	}

	void RedisSubComponent::OnConnectOK(int id)
	{
		std::unique_ptr<redis::Request> request = redis::Request::Make("SUBSCRIBE");
		auto iter = this->mChannels.begin();
		for (; iter != this->mChannels.end(); iter++)
		{
			request->AddParameter(iter->first);
		}
		request->SetRpcId(1);
		this->mClient->Send(std::move(request));
	}

	int RedisSubComponent::SubChannel(const std::string& channel)
	{
		auto iter = this->mChannels.find(channel);
		if(iter == this->mChannels.end())
		{
			this->mChannels[channel] = 0;
		}
		if(this->mChannels[channel] == 0)
		{
			this->mChannels[channel]++;
			std::unique_ptr<redis::Request> request =
					redis::Request::Make("SUBSCRIBE", channel);
			{
				request->SetRpcId(1);
				this->mClient->Send(std::move(request));
				LOG_DEBUG("sub redis channel => {}", channel);
			}
		}
		return this->mChannels[channel];
	}

	int RedisSubComponent::UnSubChannel(const std::string& channel)
	{
		auto iter = this->mChannels.find(channel);
		if(iter == this->mChannels.end())
		{
			return 0;
		}
		int count = iter->second--;
		if(iter->second <= 0)
		{
			this->mChannels.erase(iter);
			std::unique_ptr<redis::Request> request =
					redis::Request::Make("UNSUBSCRIBE", channel);
			{
				request->SetRpcId(1);
				this->mClient->Send(std::move(request));
			}
		}
		return count;
	}

	void RedisSubComponent::OnMessage(int, redis::Request* request, redis::Response* response)
	{
		do
		{
			if (response->GetType() != redis::Type::REDIS_ARRAY || response->GetArraySize() != 3)
			{
				LOG_DEBUG("{}", response->ToString());
				break;
			}
			const redis::Any* channelData = response->Get(1);
			const redis::Any* messageData = response->Get(2);
			if (!channelData->IsString() || !messageData->IsString())
			{
				break;
			}
			const std::string& channel = channelData->Cast<redis::String>()->GetValue();
			const std::string& message = messageData->Cast<redis::String>()->GetValue();

			this->mApp->Publish(channel, rpc::Porto::Json, message);
		}
		while (false);
		delete request;
		delete response;
		this->mClient->StartReceive();
	}
}