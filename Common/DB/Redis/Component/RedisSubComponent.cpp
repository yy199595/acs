//
// Created by leyi on 2024/1/18.
//

#include "XCode/XCode.h"
#include "RedisSubComponent.h"
#include "Entity/Actor/App.h"
#include "Lua/Engine/ModuleClass.h"
#include "Rpc/Component/DispatchComponent.h"
#include "Server/Component/ThreadComponent.h"
namespace acs
{
	RedisSubComponent::RedisSubComponent()
	{
		this->mDispatch = nullptr;
	}

	bool RedisSubComponent::Awake()
	{
		return ServerConfig::Inst()->Get("sub", this->mConfig);
	}

	bool RedisSubComponent::LateAwake()
	{
		const std::string & address = this->mConfig.address;
		ThreadComponent* component = this->GetComponent<ThreadComponent>();
		{
			Asio::Context & context = this->mApp->GetContext();
			tcp::Socket * sock = component->CreateSocket(address);
			this->mClient = std::make_shared<redis::Client>(sock, this->mConfig, this, context);
			if(!this->mClient->Start())
			{
				LOG_ERROR("start sub redis [{}] fail", address);
				return false;
			}
		}
		if(this->mConfig.debug)
		{
			std::unique_ptr<redis::Request> request =
					redis::Request::Make("MONITOR");
			this->mClient->Sync(std::move(request));
		}
		LOG_CHECK_RET_FALSE(this->mDispatch = this->GetComponent<DispatchComponent>())
		return true;
	}

	void RedisSubComponent::OnConnectOK(int id)
	{
		std::unique_ptr<redis::Request> request = redis::Request::Make("SUBSCRIBE");
		auto iter = this->mChannels.begin();
		for (; iter != this->mChannels.end(); iter++)
		{
			request->AddParameter(*iter);
		}
		request->SetRpcId(1);
		this->mClient->Send(std::move(request));
	}

	bool RedisSubComponent::Sub(const std::string& channel)
	{
		auto iter = this->mChannels.find(channel);
		if (iter != this->mChannels.end())
		{
			return false;
		}

//		this->mChannels.insert(channel);
//		std::unique_ptr<redis::Request> request =
//				redis::Request::Make("SUBSCRIBE", channel);
//		{
//			request->SetRpcId(1);
//			this->mClient->Send(std::move(request));
//			LOG_DEBUG("sub redis channel => {}", channel);
//		}
		return true;
	}

	bool RedisSubComponent::UnSub(const std::string& channel)
	{
		auto iter = this->mChannels.find(channel);
		if (iter == this->mChannels.end())
		{
			return false;
		}

		std::unique_ptr<redis::Request> request =
				redis::Request::Make("UNSUBSCRIBE", channel);
		{
			request->SetRpcId(1);
			this->mClient->Send(std::move(request));
		}
		this->mChannels.erase(iter);
		return true;
	}

	void RedisSubComponent::OnMessage(int, redis::Request* request, redis::Response* response) noexcept
	{
		do
		{
			const redis::Element & element = response->element;
			if (element.type != redis::type::Array || element.list.size() != 3)
			{
				LOG_DEBUG("{}", response->ToString());
				break;
			}
			const std::string& channel = element.list[1].message;
			const std::string& message = element.list[2].message;
			std::unique_ptr<rpc::Message> rpcMessage = std::make_unique<rpc::Message>();
			{
				rpcMessage->SetType(rpc::Type::Request);
				rpcMessage->SetContent(rpc::Porto::Json, message);
				rpcMessage->GetHead().Add(rpc::Header::func, channel);
			}
			if(this->mDispatch->OnMessage(rpcMessage.get()) != XCode::Ok)
			{
				break;
			}
			rpcMessage.release();
		}
		while (false);
		delete response;
		this->mClient->StartReceive();
	}
}