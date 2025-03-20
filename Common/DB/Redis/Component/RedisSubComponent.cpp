//
// Created by leyi on 2024/1/18.
//

#include "XCode/XCode.h"
#include "RedisSubComponent.h"
#include "Entity/Actor/App.h"
#include "Lua/Engine/ModuleClass.h"
#include "Rpc/Component/DispatchComponent.h"
#include "Server/Component/ThreadComponent.h"
#include "Lua/Lib/Lib.h"

namespace acs
{
	RedisSubComponent::RedisSubComponent()
	{
		this->mDispatch = nullptr;
		REGISTER_JSON_CLASS_FIELD(redis::Cluster, sub);
	}

	bool RedisSubComponent::Awake()
	{
		LuaCCModuleRegister::Add([](Lua::CCModule & ccModule) {
			ccModule.Open("db.redis.sub", lua::lib::luaopen_lsub_redisdb);
		});
		ServerConfig::Inst()->Get("redis", this->mConfig);
		LOG_CHECK_RET_FALSE(!this->mConfig.sub.empty())
		return true;
	}

	bool RedisSubComponent::LateAwake()
	{
		ThreadComponent* component = this->GetComponent<ThreadComponent>();
		{
			redis::Config redisConfig;
			if(!redisConfig.Decode(this->mConfig.sub))
			{
				return false;
			}
			redisConfig.Get("password", redisConfig.password);
			Asio::Context & context = this->mApp->GetContext();
			LOG_CHECK_RET_FALSE(redisConfig.Get("address", redisConfig.address))
			tcp::Socket * sock = component->CreateSocket(redisConfig.address);
			this->mClient = std::make_shared<redis::Client>(1, redisConfig, this, context);
			if(!this->mClient->Start(sock))
			{
				LOG_ERROR("start sub redis fail");
				return false;
			}
		}
		LOG_CHECK_RET_FALSE(this->mDispatch = this->GetComponent<DispatchComponent>())
		return true;
	}

	void RedisSubComponent::OnConnectOK(int id)
	{
		if(!this->mChannels.empty())
		{
			std::unique_ptr<redis::Request> request = redis::Request::Make("SUBSCRIBE");
			for (auto iter = this->mChannels.begin(); iter != this->mChannels.end(); iter++)
			{
				request->AddParameter(*iter);
			}
			request->SetRpcId(1);
			this->mClient->Send(std::move(request));
		}
	}

	void RedisSubComponent::Send(std::unique_ptr<redis::Request> request, int& rpcId)
	{
		rpcId = this->BuildRpcId();
		{
			request->SetRpcId(rpcId);
			this->mClient->Send(std::move(request));
		}
	}

	bool RedisSubComponent::Sub(const std::string& channel)
	{
		int rpcId = 0;
		LOG_CHECK_RET_FALSE(!channel.empty())
		std::unique_ptr<redis::Request> request = redis::Request::Make("SUBSCRIBE", channel);
		{
			this->Send(std::move(request), rpcId);
			std::unique_ptr<redis::Response> response = this->BuildRpcTask<RedisTask>(rpcId)->Await();
			return response != nullptr && response->element.type == redis::type::Array;
		}
	}

	bool RedisSubComponent::UnSub(const std::string& channel)
	{
		int rpcId = 0;
		std::unique_ptr<redis::Request> request = redis::Request::Make("UNSUBSCRIBE", channel);
		{
			this->Send(std::move(request), rpcId);
			std::unique_ptr<redis::Response> response = this->BuildRpcTask<RedisTask>(rpcId)->Await();
			return response != nullptr && response->element.type == redis::type::Array;
		}
		return true;
	}

	void RedisSubComponent::OnMessage(int, redis::Request* request, redis::Response* resp) noexcept
	{
		do
		{
			std::unique_ptr<redis::Response> response(resp);
			const redis::Element & element = response->element;
			if (element.type != redis::type::Array || element.list.size() != 3)
			{
				if(element.type == redis::type::Error)
				{
					LOG_ERROR("response => {}", response->ToString())
				}
				break;
			}
			if(request != nullptr && request->GetRpcId() > 0)
			{
				const std::string& option = element.list[0].message;
				if(option == "subscribe")
				{
					const redis::Element & element1 = element.list[1];
					const redis::Element & element2 = element.list[2];
					if(element2.number == 1)
					{
						this->mChannels.emplace(element1.message);
					}
				}
				else if(option == "unsubscribe")
				{
					const redis::Element & element1 = element.list[1];
					const redis::Element & element2 = element.list[2];
					if(element2.number == 1)
					{
						auto iter = this->mChannels.find(element1.message);
						if(iter != this->mChannels.end())
						{
							this->mChannels.erase(iter);
						}
					}
				}
				int rpcId = request->GetRpcId();
				this->OnResponse(rpcId, std::move(response));
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
			if(this->mDispatch->OnMessage(rpcMessage.get()) == XCode::Ok)
			{
				rpcMessage.release();
			}
		}
		while (false);
		this->mClient->StartReceive();
	}
}