//
// Created by mac on 2021/11/28.
//
#include "XCode/XCode.h"
#include"OuterTcpComponent.h"
#include"Rpc/Client/OuterTcpSession.h"
#include"Server/Config/CodeConfig.h"

#include"Entity/Actor/App.h"
#include"Core/Event/IEvent.h"
#include "Core/System/System.h"

namespace acs
{
	OuterTcpComponent::OuterTcpComponent()
	{
		this->mWaitCount = 0;
		this->mOuter = nullptr;
		this->mMaxConnectCount = 500;
	}

	bool OuterTcpComponent::LateAwake()
	{
		json::r::Value jsonObject;
		if(ServerConfig::Inst()->Get("connect", jsonObject))
		{
			jsonObject.Get("outer", this->mMaxConnectCount);
		}
		help::PlayerLoginEvent::Add(this, &OuterTcpComponent::OnPlayerLogin);
		help::PlayerLogoutEvent::Add(this, &OuterTcpComponent::OnPlayerLogout);
		LOG_CHECK_RET_FALSE(this->mOuter = this->mApp->GetComponent<rpc::IOuterMessage>());
		return true;
	}

	void OuterTcpComponent::OnPlayerLogin(long long userId, int sockId)
	{
		auto iter = this->mGateClientMap.find(sockId);
		if(iter != this->mGateClientMap.end())
		{
			iter->second->BindPlayer(userId);
		}
	}

	void OuterTcpComponent::OnPlayerLogout(long long userId, int sockId)
	{
		auto iter = this->mGateClientMap.find(sockId);
		if(iter != this->mGateClientMap.end())
		{
			this->mGateClientMap.erase(iter);
		}
	}

	void OuterTcpComponent::OnMessage(rpc::Message * req, rpc::Message *) noexcept
	{
		std::unique_ptr<rpc::Message> message(req);
		int code = this->mOuter->OnMessage(message);
		if(code != XCode::Ok)
		{
			int sockId = message->SockId();
			this->StartClose(sockId, code);
		}
	}

	int OuterTcpComponent::Send(int id, std::unique_ptr<rpc::Message> & message) noexcept
	{
		if (message->GetType() == rpc::type::response)
		{
			--this->mWaitCount;
		}
		auto iter = this->mGateClientMap.find(id);
		if (iter == this->mGateClientMap.end())
		{
			return XCode::SendMessageFail;
		}
		iter->second->Send(message);
		return XCode::Ok;
	}

	bool OuterTcpComponent::OnListen(tcp::Socket * socket) noexcept
	{
		if (this->mApp->GetStatus() < ServerStatus::Start)
		{
			return false;
		}

		int sockId = this->mSocketPool.BuildNumber();
		Asio::Context & io = this->mApp->GetContext();
		std::shared_ptr<rpc::OuterTcpSession> outerNetClient = std::make_shared<rpc::OuterTcpSession>(sockId, this, io);
		{
			outerNetClient->StartReceive(socket);
			this->mGateClientMap.emplace(sockId, outerNetClient);
		}
		//LOG_DEBUG("[{}] connect gate server count:{}", socket->GetAddress(), this->mGateClientMap.size())
		return true;
	}

	void OuterTcpComponent::OnClientError(int id, int code)
	{
		auto iter = this->mGateClientMap.find(id);
		if(iter != this->mGateClientMap.end())
		{
			long long playerId = iter->second->GetPlayerId();
			help::PlayerLogoutEvent::Trigger(playerId, id);
		}
		LOG_DEBUG("remove client({}) count:{}", id, this->mGateClientMap.size());
	}

	void OuterTcpComponent::OnSendFailure(int id, rpc::Message* req)
	{
		int sockId = 0;
		std::unique_ptr<rpc::Message> message(req);
		message->GetHead().Add(rpc::Header::code, XCode::SendMessageFail);
		if(message->GetType() == rpc::type::request && message->GetRpcId() > 0)
		{
			if(message->GetHead().Get(rpc::Header::sock_id, sockId))
			{
				this->Send(sockId, message);
			}
		}
	}

	void OuterTcpComponent::StartClose(int id, int code)
	{
		auto iter = this->mGateClientMap.find(id);
		if(iter != this->mGateClientMap.end())
		{
			iter->second->Stop();
			long long playerId = iter->second->GetPlayerId();
			help::PlayerLogoutEvent::Trigger(playerId, id);
		}
	}

    void OuterTcpComponent::OnRecord(json::w::Document &document)
    {
		std::unique_ptr<json::w::Value> data = document.AddObject("outer");
		{
			data->Add("wait", this->mWaitCount);
			data->Add("client", this->mGateClientMap.size());
		}
    }

	void OuterTcpComponent::Broadcast(std::unique_ptr<rpc::Message>& message) noexcept
	{
		message->SetType(rpc::type::request);
		for(auto iter = this->mGateClientMap.begin(); iter != this->mGateClientMap.end(); iter++)
		{
			if(iter->second->GetPlayerId() > 0)
			{
				continue;
			}
			std::unique_ptr<rpc::Message> broadCastMessage = message->Clone();
			{
				iter->second->Send(broadCastMessage);
			}
		}
	}
}