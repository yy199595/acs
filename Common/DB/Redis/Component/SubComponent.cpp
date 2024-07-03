//
// Created by leyi on 2024/1/18.
//

#include"SubComponent.h"
#include"Entity/Actor/App.h"
#include"Server/Component/ThreadComponent.h"

namespace joke
{
	SubController::SubController()
		: mIndex(1) { }

	bool SubController::Delete(int id)
	{
		auto iter = this->mCallbacks.find(id);
		if(iter == this->mCallbacks.end())
		{
			return false;
		}
		this->mCallbacks.erase(iter);
		return true;
	}

	int SubController::Add(joke::SubCallback&& callback)
	{
		int id = this->mIndex++;
		this->mCallbacks.emplace(id, std::move(callback));
		return id;
	}

	int SubController::OnInvoke(const std::string& json)
	{
		if(this->mCallbacks.empty())
		{
			return 0;
		}
		std::unique_ptr<json::r::Document> document(new json::r::Document());
		if(!document->Decode(json.c_str(), json.size()))
		{
			return -1;
		}
		auto iter = this->mCallbacks.begin();
		for(; iter != this->mCallbacks.end(); iter++)
		{
			iter->second(*document);
		}
		return (int)this->mCallbacks.size();
	}
}

namespace joke
{
	SubComponent::SubComponent()
	{
		this->mClient = nullptr;
	}

	bool SubComponent::Awake()
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

	bool SubComponent::LateAwake()
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
			this->mClient->Send(std::move(request));
		}
		return true;
	}

	void SubComponent::OnConnectOK(int id)
	{
		auto iter = this->mControllers.begin();
		for(; iter != this->mControllers.end(); iter++)
		{
			const std::string & channel = iter->first;
			std::unique_ptr<redis::Request> request =
					redis::Request::Make("SUBSCRIBE", channel);
			{
				request->SetRpcId(1);
				this->mClient->Send(std::move(request));
			}
		}
	}

	int SubComponent::SubChannel(const std::string& channel, SubCallback callback)
	{
		std::unique_ptr<redis::Request> request =
				redis::Request::Make("SUBSCRIBE", channel);
		{
			request->SetRpcId(1);
			this->mClient->Send(std::move(request));
		}
		auto iter = this->mControllers.find(channel);
		if(iter == this->mControllers.end())
		{
			this->mControllers.emplace(channel, new SubController());
		}
		LOG_DEBUG("sub redis channel => {}", channel)
		return this->mControllers[channel]->Add(std::move(callback));
	}

	bool SubComponent::UnSubChannel(const std::string& channel, int id)
	{
		auto iter = this->mControllers.find(channel);
		if(iter == this->mControllers.end())
		{
			return false;
		}
		iter->second->Delete(id);
		if(iter->second->Count() == 0)
		{
			std::unique_ptr<redis::Request> request =
					redis::Request::Make("UNSUBSCRIBE", channel);
			{
				request->SetRpcId(1);
				this->mClient->Send(std::move(request));
			}
		}
		return true;
	}

	void SubComponent::OnMessage(redis::Request* request, redis::Response* response)
	{
		if(request != nullptr)
		{
			delete request;
		}
		if(response->GetType() == redis::Type::REDIS_ARRAY && response->GetArraySize() == 3)
		{
			const redis::Any * channel = response->Get(1);
			const redis::Any * message = response->Get(2);
			if(channel->IsString() && message->IsString())
			{
				const std::string & str1 = channel->Cast<redis::String>()->GetValue();

				auto iter = this->mControllers.find(str1);
				if(iter != this->mControllers.end())
				{
					const std::string & str2 = message->Cast<redis::String>()->GetValue();
					iter->second->OnInvoke(str2);
				}
			}
		}
		delete response;
		this->mClient->StartReceive();
	}
}