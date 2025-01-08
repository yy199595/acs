//
// Created by 64658 on 2025/1/2.
//

#include "XCode/XCode.h"
#include "Entity/Actor/App.h"
#include "InnerWebSocketComponent.h"
#include "WebSocket/Common/WebSocketMessage.h"
#include "WebSocket/Client/WebSocketClient.h"
#include "WebSocket/Client/WebSocketSessionClient.h"
#include "Server/Component/ThreadComponent.h"
#include "Rpc/Component/DispatchComponent.h"
namespace acs
{
	InnerWebSocketComponent::InnerWebSocketComponent()
		: mClientPool(SERVER_MAX_COUNT)
	{
		this->mActor = nullptr;
		this->mThread = nullptr;
		this->mDispatch = nullptr;
	}

	bool InnerWebSocketComponent::OnListen(tcp::Socket* socket) noexcept
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

	bool InnerWebSocketComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mActor = this->GetComponent<ActorComponent>())
		LOG_CHECK_RET_FALSE(this->mThread = this->GetComponent<ThreadComponent>())
		LOG_CHECK_RET_FALSE(this->mDispatch = this->GetComponent<DispatchComponent>())
		return true;
	}

	void InnerWebSocketComponent::OnMessage(int id, rpc::Message* request, rpc::Message* response) noexcept
	{
		if (this->mDispatch->OnMessage(request) == XCode::Ok)
		{
			return;
		}
		LOG_DEBUG("[{}] message = {}", id, request->ToString())
		delete request;
	}

	int InnerWebSocketComponent::Send(int id, rpc::Message* message) noexcept
	{
		auto iter = this->mSessions.find(id);
		if(iter != this->mSessions.end())
		{
			iter->second->Send(message);
			return XCode::Ok;
		}
		auto iter1 = this->mClients.find(id);
		if(iter1 != this->mClients.end())
		{
			iter1->second->Send(message);
			return XCode::Ok;
		}
		std::string address;
		if(!this->mActor->GetListen(id, "ws", address))
		{
			return XCode::NotFoundActorAddress;
		}
		Asio::Context & context = this->mApp->GetContext();
		tcp::Socket * tcpSocket = this->mThread->CreateSocket();
		std::shared_ptr<ws::RequestClient> requestClient = std::make_shared<ws::RequestClient>(id, this, context);
		{
			tcpSocket->Init(address);
			requestClient->SetSocket(tcpSocket);
			this->mClients.emplace(id, requestClient);
		}
		requestClient->Send(message);
		return XCode::SendMessageFail;
	}

	int InnerWebSocketComponent::Send(int id, ws::Message* message) noexcept
	{
		auto iter = this->mSessions.find(id);
		if(iter != this->mSessions.end())
		{
			iter->second->Send(message);
			return XCode::Ok;
		}
		auto iter1 = this->mClients.find(id);
		if(iter1 == this->mClients.end())
		{
			return XCode::SendMessageFail;
		}
		iter1->second->Send(message);
		return XCode::Ok;
	}


	int InnerWebSocketComponent::Create(const std::string& address)
	{
		int id = this->mClientPool.BuildNumber();
		Asio::Context & context = this->mApp->GetContext();
		tcp::Socket * tcpSocket = this->mThread->CreateSocket();
		std::shared_ptr<ws::RequestClient> requestClient = std::make_shared<ws::RequestClient>(id, this, context);
		{
			tcpSocket->Init(address);
			requestClient->SetSocket(tcpSocket);
			this->mClients.emplace(id, requestClient);
		}
		return id;
	}


	void InnerWebSocketComponent::OnClientError(int id, int code)
	{
		auto iter = this->mSessions.find(id);
		if(iter != this->mSessions.end())
		{
			this->mSessions.erase(iter);
			LOG_WARN("remove ws client id:{} code=>{}", id, code);
		}
	}
}