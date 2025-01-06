//
// Created by 64658 on 2025/1/2.
//

#include "XCode/XCode.h"
#include "Entity/Actor/App.h"
#include "OuterWebSocketComponent.h"
#include "Gate/Component/GateComponent.h"
#include "WebSocket/Common/WebSocketMessage.h"
#include "WebSocket/Client/WebSocketClient.h"
#include "WebSocket/Client/WebSocketSessionClient.h"

namespace acs
{
	OuterWebSocketComponent::OuterWebSocketComponent()
			: mClientPool(SERVER_MAX_COUNT)
	{
		this->mGate = nullptr;
	}

	bool OuterWebSocketComponent::OnListen(tcp::Socket* socket)
	{
		int id = this->mClientPool.BuildNumber();
		Asio::Context & io = this->mApp->GetContext();
		std::shared_ptr<ws::SessionClient> sessionClient
				= std::make_unique<ws::SessionClient>(id, this, io);
		{
			sessionClient->StartReceive(socket);
			this->mSessions.emplace(id, sessionClient);
		}
		return true;
	}

	bool OuterWebSocketComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mGate = this->GetComponent<GateComponent>())
		return true;
	}

	void OuterWebSocketComponent::OnMessage(int id, rpc::Message* request, rpc::Message* response)
	{
		int code = this->mGate->OnMessage(request);
		if(code != XCode::Ok)
		{
			auto iter = this->mSessions.find(id);
			if(iter != this->mSessions.end())
			{
				iter->second->Stop();
				this->mSessions.erase(iter);
			}
		}
	}

	void OuterWebSocketComponent::Broadcast(rpc::Message* message)
	{

	}

	int OuterWebSocketComponent::Send(int id, rpc::Message* message)
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
}