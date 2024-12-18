//
// Created by mac on 2021/11/28.
//

#include"OuterNetComponent.h"
#include"Gate/Client/OuterClient.h"
#include"Server/Config/CodeConfig.h"

#include"Gate/Service/GateSystem.h"
#include"Entity/Actor/App.h"
#include"Entity/Actor/Player.h"
#include"Core/Event/IEvent.h"
#include"Router/Component/RouterComponent.h"

namespace acs
{
	OuterNetComponent::OuterNetComponent()
	{
		this->mWaitCount = 0;
		this->mRouter = nullptr;
		this->mActComponent = nullptr;
		this->mMaxConnectCount = 500;
	}

	bool OuterNetComponent::LateAwake()
	{
		std::unique_ptr<json::r::Value> jsonObject;
		if(ServerConfig::Inst()->Get("connect", jsonObject))
		{
			jsonObject->Get("outer", this->mMaxConnectCount);
		}
		this->mRouter = this->GetComponent<RouterComponent>();
		this->mActComponent = this->GetComponent<ActorComponent>();
		help::PlayerLoginEvent::Add(this, &OuterNetComponent::OnPlayerLogin);
		help::PlayerLogoutEvent::Add(this, &OuterNetComponent::OnPlayerLogout);
		return true;
	}

	void OuterNetComponent::OnPlayerLogin(long long userId, int sockId)
	{
		auto iter = this->mGateClientMap.find(sockId);
		if(iter != this->mGateClientMap.end())
		{
			iter->second->BindPlayer(userId);
			LOG_DEBUG("[{}] user({}) login ok", this->mGateClientMap.size(), userId);
		}
	}

	void OuterNetComponent::OnPlayerLogout(long long userId, int sockId)
	{
		auto iter = this->mGateClientMap.find(sockId);
		if(iter != this->mGateClientMap.end())
		{
			this->mGateClientMap.erase(iter);
			Player * player = this->mActComponent->GetPlayer(userId);
			if(player != nullptr)
			{
				player->Logout();
				this->mActComponent->DelActor(userId);
				LOG_WARN("[{}] user({}) logout ok", this->mGateClientMap.size(), userId);
			}
		}
	}

	void OuterNetComponent::OnMessage(rpc::Packet * message, rpc::Packet *)
	{
		int code = XCode::Failure;
		message->SetNet(rpc::Net::Tcp);
		switch (message->GetType())
		{
			case rpc::Type::Request:
			{
//#ifdef __DEBUG__
//				std::string func = message->GetHead().GetStr(rpc::Header::func);
//				LOG_DEBUG("user({}) call [{}]", playerId, func);
//#endif
				if (message->GetRpcId() > 0)
				{
					++this->mWaitCount;
					message->SetRpcId(this->mNumPool.BuildNumber());
				}
				code = this->OnRequest(message);
				break;
			}
			case rpc::Type::Response:
			{
				int sockId = 0;
				--this->mWaitCount;
				if (message->GetHead().Del(rpc::Header::client_sock_id, sockId))
				{
					int rpcId = 0;
					if (message->GetHead().Del(rpc::Header::rpc_id, rpcId))
					{
						code = XCode::Ok;
						message->SetRpcId(rpcId);
						this->SendBySockId(sockId, message);
					}
				}
				break;
			}
			default:
			LOG_ERROR("unknown message {}", message->ToString());
				break;
		}
		if (code != XCode::Ok)
		{
			this->StartClose(message->SockId(), code);
#ifdef __DEBUG__
			const std::string& desc = CodeConfig::Inst()->GetDesc(code);
			//LOG_WARN("{}\n desc={}", message->ToString(), desc);
#endif
			delete message;
		}
	}

	int OuterNetComponent::OnRequest(rpc::Packet * message)
	{
		const std::string& fullName = message->GetHead().GetStr(rpc::Header::func);
		const RpcMethodConfig* methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
		if (methodConfig == nullptr || !methodConfig->IsClient || !methodConfig->IsOpen)
		{
			LOG_ERROR("call function not exist : {}", fullName)
			return XCode::CallFunctionNotExist;
		}
		long long playerId = 0;
		message->SetNet(rpc::Net::Tcp);
		int serverId = this->mApp->GetSrvId();
		message->GetHead().Get(rpc::Header::player_id, playerId);
		Player* player = this->mActComponent->GetPlayer(playerId);
		if (player == nullptr)
		{
			if(methodConfig->IsAuth)
			{
				int sockId = message->SockId();
				this->StartClose(sockId, XCode::NotFindUser);
				return XCode::CloseSocket;
			}
			this->mRouter->Send(serverId, std::unique_ptr<rpc::Packet>(message));
			return XCode::Ok;
		}
		const std::string& name = methodConfig->Server;
		switch (methodConfig->Forward)
		{
			case rpc::Forward::Fixed: //转发到固定机器
				if (!player->GetServerId(name, serverId))
				{
					return XCode::NotFoundServerRpcAddress;
				}
				break;
			case rpc::Forward::Random:
			{
				Server* server = this->mActComponent->Random(name);
				if (server == nullptr)
				{
					return XCode::AddressAllotFailure;
				}
				if (!server->GetAddress(*message, serverId))
				{
					return XCode::NotFoundActor;
				}
				break;
			}
			case rpc::Forward::Hash:
			{
				Server* server = this->mActComponent->Hash(name, playerId);
				if (server == nullptr)
				{
					return XCode::AddressAllotFailure;
				}
				if (!server->GetAddress(*message, serverId))
				{
					return XCode::NotFoundActor;
				}
				break;
			}
			default:
			LOG_ERROR("unknown forward {}", message->ToString());
				return XCode::Failure;
		}
		this->mRouter->Send(serverId, std::unique_ptr<rpc::Packet>(message));
		return XCode::Ok;
	}

	bool OuterNetComponent::SendBySockId(int id, rpc::Packet * message)
	{
		do
		{
			auto iter = this->mGateClientMap.find(id);
			if (iter == this->mGateClientMap.end())
			{
				break;
			}
			if (iter->second->Send(message))
			{
				return true;
			}
		} while (false);
//#if __DEBUG__
//		LOG_ERROR("send to client({}) =>{}", id, message->ToString())
//#endif
		delete message;
		return false;
	}

	bool OuterNetComponent::OnListen(tcp::Socket * socket)
	{
		if (this->mApp->GetStatus() < ServerStatus::Start)
		{
			return false;
		}

		int sockId = this->mSocketPool.BuildNumber();
		Asio::Context & io = this->mApp->GetContext();
		std::shared_ptr<rpc::OuterClient> outerNetClient = std::make_shared<rpc::OuterClient>(sockId, this, io);
		{
			outerNetClient->StartReceive(socket);
			this->mGateClientMap.emplace(sockId, outerNetClient);
		}
		//LOG_DEBUG("[{}] connect gate server count:{}", socket->GetAddress(), this->mGateClientMap.size())
		return true;
	}

	void OuterNetComponent::OnClientError(int id, int code)
	{
		auto iter = this->mGateClientMap.find(id);
		if(iter != this->mGateClientMap.end())
		{
			long long playerId = iter->second->GetPlayerId();
			help::PlayerLogoutEvent::Trigger(playerId, id);
		}
		LOG_DEBUG("remove client({}) count:{}", id, this->mGateClientMap.size());
	}

	void OuterNetComponent::OnSendFailure(int id, rpc::Packet* message)
	{
		int sockId = 0;
		std::unique_ptr<rpc::Packet> request(message);
		message->GetHead().Add(rpc::Header::code, XCode::SendMessageFail);
		if(message->GetType() == rpc::Type::Request && message->GetRpcId() > 0)
		{
			if(message->GetHead().Get(rpc::Header::sock_id, sockId))
			{
				this->SendBySockId(sockId, request.release());
			}
		}
	}

	void OuterNetComponent::StartClose(int id, int code)
	{
		auto iter = this->mGateClientMap.find(id);
		if(iter != this->mGateClientMap.end())
		{
			iter->second->Stop();
			long long playerId = iter->second->GetPlayerId();
			help::PlayerLogoutEvent::Trigger(playerId, id);
		}
	}

    void OuterNetComponent::OnRecord(json::w::Document &document)
    {
		std::unique_ptr<json::w::Value> data = document.AddObject("outer");
		{
			data->Add("wait", this->mWaitCount);
			data->Add("client", this->mGateClientMap.size());
			data->Add("sum", this->mNumPool.CurrentNumber());
		}
    }

	void OuterNetComponent::Broadcast(rpc::Packet * message)
	{
		message->SetType(rpc::Type::Request);
		for(auto iter = this->mGateClientMap.begin(); iter != this->mGateClientMap.end(); iter++)
		{
			if(iter->second->GetPlayerId() == 0)
			{
				continue;
			}
			std::unique_ptr<rpc::Packet> broadCastMessage = message->Clone();
			{
				iter->second->Send(broadCastMessage.release());
			}
		}
		delete message;
	}
}