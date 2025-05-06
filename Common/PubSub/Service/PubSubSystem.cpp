//
// Created by 64658 on 2024/9/2.
//

#include "PubSubSystem.h"
#include "Entity/Actor/App.h"
#include "Rpc/Component/DispatchComponent.h"
#include "Router/Component/RouterComponent.h"
namespace acs
{
	PubSubSystem::PubSubSystem()
	{
		this->mRouter = nullptr;
	}

	bool PubSubSystem::OnInit()
	{
		BIND_RPC_METHOD(PubSubSystem::Publish);
		BIND_RPC_METHOD(PubSubSystem::Subscribe);
		BIND_RPC_METHOD(PubSubSystem::UbSubscribe);
		this->mRouter = this->GetComponent<RouterComponent>();
		return true;
	}

	int PubSubSystem::Subscribe(const rpc::Message& request)
	{
		int nodeId = 0;
		const rpc::Head & head = request.ConstHead();
		const std::string & channel = request.GetBody();
		LOG_ERROR_CHECK_ARGS(head.Get(rpc::Header::app_id, nodeId));

		auto iter = this->mSubInfos.find(nodeId);
		if(iter == this->mSubInfos.end())
		{
			sub::Client subClient;
			subClient.nodeId = nodeId;
			subClient.sockId = request.SockId();
			subClient.channel.emplace(channel);
			this->mSubInfos.emplace(subClient.nodeId, subClient);
			return XCode::Ok;
		}
		iter->second.channel.emplace(channel);
		return XCode::Ok;
	}

	int PubSubSystem::UbSubscribe(const rpc::Message& request)
	{
		int nodeId = 0;
		const rpc::Head & head = request.ConstHead();
		const std::string & channel = request.GetBody();
		LOG_ERROR_CHECK_ARGS(head.Get(rpc::Header::app_id, nodeId));
		auto iter = this->mSubInfos.find(nodeId);
		if(iter == this->mSubInfos.end())
		{
			return XCode::Failure;
		}
		sub::Client & subClient = iter->second;
		auto iter1 = subClient.channel.find(channel);
		if(iter1 == subClient.channel.end())
		{
			return XCode::Failure;
		}
		if(subClient.channel.empty())
		{
			this->mSubInfos.erase(iter);
		}
		return XCode::Ok;
	}

	int PubSubSystem::Publish(const json::r::Document & document)
	{
		int count = 0;
		std::string channel;
		std::string message;
		char proto = rpc::Proto::String;
		LOG_ERROR_CHECK_ARGS(document.Get("channel", channel));
		if(!document.Get("message", message))
		{
			std::unique_ptr<json::r::Value> jsonValue;
			if(!document.Get("message", jsonValue))
			{
				return XCode::CallArgsError;
			}
			proto = rpc::Proto::Json;
			message = jsonValue->ToString();
		}
		for(auto iter = this->mSubInfos.begin(); iter != this->mSubInfos.end(); iter++)
		{
			sub::Client & subClient = iter->second;
			for(const std::string & subChannel : subClient.channel)
			{
				if(subChannel == channel)
				{
					std::unique_ptr<rpc::Message> rpcMessage = this->mApp->Make(channel);
					if(rpcMessage != nullptr)
					{
						rpcMessage->SetContent(proto, message);
						rpcMessage->SetSockId(subClient.sockId);
						//subClient.messages.emplace(std::move(rpcMessage));
						if(this->mRouter->Send(subClient.sockId, std::move(rpcMessage)) == XCode::Ok)
						{
							count++;
						}
					}
				}
			}
		}
		return XCode::Ok;
	}
}