//
// Created by mac on 2021/11/28.
//

#include"OuterNetComponent.h"
#include"Gate/Client/OuterClient.h"
#include"Server/Config/CodeConfig.h"

#include"Gate/Service/GateSystem.h"
#include"Entity/Actor/App.h"
#include"Entity/Actor/Player.h"
#include"Server/Component/ThreadComponent.h"
#include"Rpc/Client/InnerClient.h"
#include"Core/Event/IEvent.h"
#include"Rpc/Component/DispatchComponent.h"

namespace acs
{
	OuterNetComponent::OuterNetComponent()
	{
		this->mWaitCount = 0;
		this->mActComponent = nullptr;
		this->mMaxConnectCount = 500;
		this->mThreadComponent = nullptr;
	}

	bool OuterNetComponent::LateAwake()
	{
		std::unique_ptr<json::r::Value> jsonObject;
		if(ServerConfig::Inst()->Get("connect", jsonObject))
		{
			jsonObject->Get("outer", this->mMaxConnectCount);
		}
		this->mActComponent = this->GetComponent<ActorComponent>();
		this->mThreadComponent = this->GetComponent<ThreadComponent>();
		return true;
	}

	void OuterNetComponent::OnTimeout(int id)
	{
		this->OnCloseSocket(id, XCode::NetTimeout);
	}

	void OuterNetComponent::OnMessage(rpc::Packet * message, rpc::Packet *)
	{
		int code = XCode::Failure;
		message->SetNet(rpc::Net::Tcp);
		switch (message->GetType())
		{
			case rpc::Type::Request:
			{
				long long playerId = -1;
				int id = message->SockId();
				int rpcId = message->GetRpcId();
				if (!this->mAddressUserMap.Get(id, playerId))
				{
					message->GetHead().Add(rpc::Header::sock_id, id);
				}
				message->GetHead().Add(rpc::Header::player_id, playerId);
//#ifdef __DEBUG__
//				std::string func = message->GetHead().GetStr(rpc::Header::func);
//				LOG_DEBUG("user({}) call [{}]", playerId, func);
//#endif
				if (rpcId > 0)
				{
					++this->mWaitCount;
					message->TempHead().Add(rpc::Header::client_id, id);
					message->TempHead().Add(rpc::Header::rpc_id, rpcId);
				}
				code = this->OnRequest(playerId, message);
				break;
			}
			case rpc::Type::Response:
			{
				int rpcId = 0;
				if(message->TempHead().Del(rpc::Header::rpc_id, rpcId))
				{
					--this->mWaitCount;
					message->SetRpcId(rpcId);
					if(message->TempHead().Del(rpc::Header::client_id, rpcId))
					{
						code = XCode::Ok;
						this->Send(rpcId, message);
					}
				}
				break;
			}
			default:
				LOG_ERROR("unknown message {}", message->ToString());
				break;
		}
		if(code != XCode::Ok)
		{
			delete message;
		}
	}

	int OuterNetComponent::OnRequest(long long playerId, rpc::Packet * message)
	{
		const std::string& fullName = message->GetHead().GetStr(rpc::Header::func);
		const RpcMethodConfig* methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
		if (methodConfig == nullptr || !methodConfig->IsClient || !methodConfig->IsOpen)
		{
			LOG_ERROR("call function not exist : {}", fullName)
			return XCode::CallFunctionNotExist;
		}

		Player * player = this->mActComponent->GetPlayer(playerId);
		if(player == nullptr)
		{
			return this->Forward(this->mApp->GetSrvId(), message);
		}

		int serverId = 0;
		const std::string & name = methodConfig->Server;
		switch(methodConfig->Forward)
		{
			case rpc::Forward::Fixed: //转发到固定机器
				if (!player->GetServerId(name, serverId))
				{
					return XCode::NotFoundServerRpcAddress;
				}
				break;
			case rpc::Forward::Random:
			{
				Server * server = this->mActComponent->Random(name);
				if(server == nullptr)
				{
					return XCode::AddressAllotFailure;
				}
				if(!server->GetAddress(*message, serverId))
				{
					return XCode::NotFoundActor;
				}
				break;
			}
			case rpc::Forward::Hash:
			{
				Server * server = this->mActComponent->Hash(name, playerId);
				if(server == nullptr)
				{
					return XCode::AddressAllotFailure;
				}
				if(!server->GetAddress(*message, serverId))
				{
					return XCode::NotFoundActor;
				}
				break;
			}
			default:
			LOG_ERROR("unknown forward {}", message->ToString());
				return XCode::Failure;
		}
		return this->Forward(serverId, message);
	}

	//转发到内网
	int OuterNetComponent::Forward(int id, rpc::Packet * message)
	{
		rpc::InnerClient * tcpClient = nullptr;
		do
		{
			if (this->mForwardClientMap.Get(id, tcpClient))
			{
				break;
			}
			std::string address;
			static const std::string rpc("rpc");
			if (!this->mActComponent->GetListen(id, rpc, address))
			{
				return XCode::NotFoundActorAddress;
			}
			tcp::Socket* socketProxy = this->mThreadComponent->CreateSocket(address);
			if (socketProxy == nullptr)
			{
				LOG_ERROR("create socket fail : {}", address)
				break;
			}
			tcpClient = new rpc::InnerClient(id, this);
			{
				tcpClient->SetSocket(socketProxy);
				this->mForwardClientMap.Add(id, tcpClient);
			}
		}
		while(false);

		if(tcpClient == nullptr)
		{
			return XCode::SendMessageFail;
		}
		message->SetRpcId(this->mNumPool.BuildNumber());
		return tcpClient->Send(message) ? XCode::Ok : XCode::NetWorkError;
	}

	bool OuterNetComponent::Send(int id, rpc::Packet * message)
	{
		auto iter = this->mGateClientMap.find(id);
		if(iter == this->mGateClientMap.end())
		{
			delete message;
			return false;
		}

		if(!iter->second->Send(message))
		{
			delete message;
			return false;
		}
		return true;
	}

	bool OuterNetComponent::AddPlayer(long long userId, int sockId)
	{
		if(!this->mUserAddressMap.Add(userId, sockId))
		{
			return false;
		}
		this->mAddressUserMap.Add(sockId, userId);
		return true;
	}

	bool OuterNetComponent::Send(int id, int code, rpc::Packet * message)
	{
		if (message->GetRpcId() == 0)
		{
			delete message;
			return false;
		}
		message->Clear();
		message->SetType(rpc::Type::Response);
		message->GetHead().Del(rpc::Header::func);
		message->GetHead().Add(rpc::Header::code, code);
		return this->Send(message->SockId(), message);
	}

	bool OuterNetComponent::OnListen(tcp::Socket * socket)
	{
		if (this->mApp->GetStatus() < ServerStatus::Start)
		{
			return false;
		}
		int id = this->mSocketPool.BuildNumber();
		std::unique_ptr<rpc::OuterClient> outerNetClient = std::make_unique<rpc::OuterClient>(id, this);
		{
			outerNetClient->StartReceive(socket);
			this->mGateClientMap.emplace(id, std::move(outerNetClient));
		}
		LOG_DEBUG("[{}] connect gate server count:{}", socket->GetAddress(), this->mGateClientMap.size())
		return true;
	}

	void OuterNetComponent::OnCloseSocket(int id, int code)
    {
		long long userId = 0;
		if(this->mAddressUserMap.Del(id, userId))
		{
			this->mUserAddressMap.Del(userId);
			help::OuterLogoutEvent::Trigger(userId);
		}
		auto iter = this->mGateClientMap.find(id);
		if(iter != this->mGateClientMap.end())
		{
			this->mGateClientMap.erase(iter);
		}
		LOG_DEBUG("remove client({}) count:{}", id, this->mGateClientMap.size());
	}

    bool OuterNetComponent::SendToPlayer(long long userId, rpc::Packet * message)
    {
		int socketId = 0;
		if(!this->mUserAddressMap.Get(userId, socketId))
		{
			delete message;
			LOG_ERROR("send message to ({}) fail : {}", userId, message->ToString());
			return false;
		}
		LOG_DEBUG("send client({}) : {}", userId, message->ToString());
		return this->Send(socketId, message);
    }

	void OuterNetComponent::StartClose(int id, int code)
	{
		auto iter = this->mGateClientMap.find(id);
		if(iter != this->mGateClientMap.end())
		{
			iter->second->Stop(code);
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
		auto iter = this->mAddressUserMap.Begin();
		for(; iter != this->mAddressUserMap.End(); iter++)
		{
			std::unique_ptr<rpc::Packet> request = message->Clone();
			{
				this->Send(iter->first, request.release());
			}
		}
		delete message;
	}

	bool OuterNetComponent::StopClient(long long userId)
	{
		int id = 0;
		if(!this->mUserAddressMap.Get(userId,  id))
		{
			return false;
		}
		this->mAddressUserMap.Del(id);
		this->StartClose(id, XCode::NetActiveShutdown);
		return true;
	}
}