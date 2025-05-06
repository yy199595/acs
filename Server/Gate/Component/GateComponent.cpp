//
// Created by leyi on 2023/9/11.
//
#include "XCode/XCode.h"
#include "GateComponent.h"
#include "Entity/Actor/App.h"
#include "Common/Entity/Player.h"
#include "Util/Tools/TimeHelper.h"
#include "Rpc/Config/MethodConfig.h"
#include "Node/Component/NodeComponent.h"
#include "Common/Component/PlayerComponent.h"
#include "Router/Component/RouterComponent.h"


namespace acs
{
	GateComponent::GateComponent()
	{
		this->mNode = nullptr;
		this->mRouter = nullptr;
		this->mOuterComponent = nullptr;
		this->mPlayerComponent = nullptr;
	}

	bool GateComponent::LateAwake()
	{
		std::vector<rpc::IOuterSender *> outerSenders;
		LOG_CHECK_RET_FALSE(this->mApp->GetComponents(outerSenders) == 1);
		LOG_CHECK_RET_FALSE(this->mNode = this->GetComponent<NodeComponent>())
		LOG_CHECK_RET_FALSE(this->mRouter = this->GetComponent<RouterComponent>())
		LOG_CHECK_RET_FALSE(this->mPlayerComponent = this->GetComponent<PlayerComponent>())
		LOG_CHECK_RET_FALSE(this->mOuterComponent = this->mApp->GetComponent<rpc::IOuterSender>())
		return true;
	}

	int GateComponent::Send(int id, rpc::Message* message)
	{
		if(this->mOuterComponent->Send(id, message) != XCode::Ok)
		{
			delete message;
			return XCode::SendMessageFail;
		}
		return XCode::Ok;
	}

	void GateComponent::Broadcast(rpc::Message* message)
	{

	}

	int GateComponent::OnMessage(rpc::Message* message) noexcept
	{
#ifdef __DEBUG__
		rpc::Head& head = message->GetHead();
		long long nowTime = help::Time::NowMil();
#endif
		int code = XCode::Failure;
		switch (message->GetType())
		{
			case rpc::Type::Request:
#ifdef __DEBUG__
				head.Add("t1", nowTime);
#endif
				code = this->OnRequest(message);
				break;
			case rpc::Type::Response:
			{
#ifdef __DEBUG__
				std::string func;
				long long startTime = 0;
				head.Get("t1", startTime);
				head.Get(rpc::Header::func, func);
				LOG_INFO("call [{}] use time => {}ms", func, nowTime - startTime);
#endif
				code = this->OnResponse(message);
			}
				break;
			default:
				code = XCode::UnKnowPacket;
				LOG_ERROR("unknown message {}", message->ToString());
				break;
		}
		if (code != XCode::Ok)
		{
			delete message;
			return code;
		}
		return XCode::Ok;
	}

	int GateComponent::OnRequest(rpc::Message* message)
	{
		char net = message->GetNet();
		message->SetNet(rpc::Net::Tcp);
		message->GetHead().Add("n", net);
		message->SetSource(rpc::Source::Client);
		assert(message->GetHead().Has(rpc::Header::client_sock_id));
		const std::string& fullName = message->GetHead().GetStr(rpc::Header::func);
		const RpcMethodConfig* methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
		if (methodConfig == nullptr || !methodConfig->client || !methodConfig->open)
		{
			LOG_ERROR("call function not exist : {}", fullName)
			return XCode::CallFunctionNotExist;
		}
		long long playerId = 0;
		message->SetNet(rpc::Net::Tcp);
		int serverId = this->mApp->GetNodeId();
		message->GetHead().Get(rpc::Header::id, playerId);
		Player* player = this->mPlayerComponent->Get(playerId);
		if (player == nullptr)
		{
			if(methodConfig->auth)
			{
				return XCode::CloseSocket;
			}
			this->mRouter->Send(serverId, std::unique_ptr<rpc::Message>(message));
			return XCode::Ok;
		}
		const std::string& name = methodConfig->server;
		NodeCluster * nodeCluster = this->mNode->GetCluster(name);
		if(nodeCluster == nullptr)
		{
			return XCode::NotFoundActor;
		}
		switch (methodConfig->forward)
		{
			case rpc::Forward::Fixed: //转发到固定机器
				if (!player->GetServerId(name, serverId))
				{
					return XCode::NotFoundServerRpcAddress;
				}
				break;
			case rpc::Forward::Random:
			{
				if(!nodeCluster->Random(serverId))
				{
					return XCode::NotFoundActor;
				}
				break;
			}
			case rpc::Forward::Hash:
			{
				if(!nodeCluster->Hash(playerId, serverId))
				{
					return XCode::NotFoundActor;
				}
				break;
			}
			case rpc::Forward::Next:
			{
				if(!nodeCluster->Next(serverId))
				{
					return XCode::NotFoundActor;
				}
				break;
			}
			default:
			LOG_ERROR("unknown forward {}", message->ToString());
				return XCode::Failure;
		}
		this->mRouter->Send(serverId, std::unique_ptr<rpc::Message>(message));
		return XCode::Ok;
	}

	int GateComponent::OnResponse(rpc::Message* message)
	{
		int socketId, netType = 0;
		if (!message->GetHead().Del("n", netType))
		{
			return XCode::UnKnowPacket;
		}
		if (!message->GetHead().Del(rpc::Header::client_sock_id, socketId))
		{
			return XCode::UnKnowPacket;
		}
		message->SetNet((char)netType);
		message->SetSource(rpc::Source::Server);

		this->Send(socketId, message);
		return XCode::Ok;
	}
}