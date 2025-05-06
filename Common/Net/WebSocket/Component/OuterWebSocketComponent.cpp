//
// Created by 64658 on 2025/1/2.
//

#include "XCode/XCode.h"
#include "Entity/Actor/App.h"
#include "OuterWebSocketComponent.h"
#include "WebSocket/Common/WebSocketMessage.h"
#include "WebSocket/Client/Client.h"
#include "WebSocket/Client/Session.h"
#include "Core/Event/IEvent.h"

constexpr char msg_format = rpc::msg::json;

namespace acs
{
	OuterWebSocketComponent::OuterWebSocketComponent()
			: mClientPool(SERVER_MAX_COUNT)
	{
		this->mSumCount = 0;
		this->mOuter = nullptr;
	}

	bool OuterWebSocketComponent::OnListen(tcp::Socket* socket) noexcept
	{
		int id = this->mClientPool.BuildNumber();
		Asio::Context & io = this->mApp->GetContext();
		std::shared_ptr<ws::Session> sessionClient
				= std::make_unique<ws::Session>(id, this, io, msg_format);
		{
			sessionClient->StartReceive(socket);
			this->mSessions.emplace(id, sessionClient);
		}
		return true;
	}


	bool OuterWebSocketComponent::LateAwake()
	{
		help::PlayerLoginEvent::Add(this, &OuterWebSocketComponent::OnPlayerLogin);
		help::PlayerLogoutEvent::Add(this, &OuterWebSocketComponent::OnPlayerLogout);
		LOG_CHECK_RET_FALSE(this->mOuter = this->mApp->GetComponent<rpc::IOuterMessage>())
		return true;
	}

	void OuterWebSocketComponent::OnMessage(int id, rpc::Message* request, rpc::Message* response) noexcept
	{
		++this->mSumCount;
		if(this->mOuter->OnMessage(request) != XCode::Ok)
		{
			auto iter = this->mSessions.find(id);
			if(iter != this->mSessions.end())
			{
				iter->second->Stop();
			}
		}
	}

	void OuterWebSocketComponent::Broadcast(rpc::Message* message) noexcept
	{

	}

	int OuterWebSocketComponent::Send(int id, rpc::Message* message) noexcept
	{
		auto iter = this->mSessions.find(id);
		if(iter != this->mSessions.end())
		{
			iter->second->Send(message);
			return XCode::Ok;
		}
		return XCode::SendMessageFail;
	}

	void OuterWebSocketComponent::OnClientError(int id, int code)
	{
		auto iter = this->mSessions.find(id);
		if(iter != this->mSessions.end())
		{
			this->mSessions.erase(iter);
			LOG_WARN("remove ws client id:{} code=>{}", id, code);
		}
	}

	void OuterWebSocketComponent::OnPlayerLogin(long long userId, int sockId)
	{
		auto iter = this->mSessions.find(sockId);
		if(iter != this->mSessions.end())
		{
			iter->second->BindPlayerID(userId);
		}
	}

	void OuterWebSocketComponent::OnPlayerLogout(long long userId, int sockId)
	{
		auto iter = this->mSessions.find(sockId);
		if(iter != this->mSessions.end())
		{
			iter->second->Stop();
			this->mSessions.erase(iter);
		}
	}

	void OuterWebSocketComponent::OnRecord(json::w::Document& document)
	{
		std::unique_ptr<json::w::Value> jsonObject = document.AddObject("ws");
		{
			jsonObject->Add("sum", this->mSumCount);
			jsonObject->Add("client", this->mSessions.size());
		}
	}
}