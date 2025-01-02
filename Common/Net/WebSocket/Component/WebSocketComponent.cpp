//
// Created by 64658 on 2025/1/2.
//

#include "XCode/XCode.h"
#include "Entity/Actor/App.h"
#include "WebSocketComponent.h"
#include "WebSocket/Common/WebSocketMessage.h"
#include "WebSocket/Client/WebSocketClient.h"
#include "WebSocket/Client/WebSocketSessionClient.h"
#include "Server/Component/ThreadComponent.h"
namespace acs
{
	WebSocketComponent::WebSocketComponent()
		: ISender(rpc::Net::Ws)
	{
		this->mActor = nullptr;
		this->mThread = nullptr;
	}

	bool WebSocketComponent::OnListen(tcp::Socket* socket)
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

	void WebSocketComponent::Complete()
	{
		rpc::Message * rpcMessage = new rpc::Message();
		{
			rpcMessage->SetContent("11223344");
			rpcMessage->GetHead().Add("func", "ChatSystem.Ping");
			this->Send(this->mApp->GetSrvId(), rpcMessage);
		}
	}

	bool WebSocketComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mActor = this->GetComponent<ActorComponent>())
		LOG_CHECK_RET_FALSE(this->mThread = this->GetComponent<ThreadComponent>())
		return true;
	}

	void WebSocketComponent::OnMessage(int id, ws::Message* request, ws::Message* response)
	{
		LOG_DEBUG("[{}] message = {}", id, request->GetMessageBody())
		std::unique_ptr<rpc::Message> rpcMessage = std::make_unique<rpc::Message>();
		{
			rpcMessage->SetType(rpc::Type::Response);
			rpcMessage->SetProto(rpc::Porto::String);
			rpcMessage->SetContent("server response");
			this->Send(id, rpcMessage.release());
		}
		delete request;
	}

	int WebSocketComponent::Send(int id, rpc::Message* message)
	{
		std::unique_ptr<ws::Message> wsMessage = std::make_unique<ws::Message>();
		{
			std::string json;
			message->EncodeToJson(json);
			wsMessage->SetBody(ws::OPCODE_TEXT, json);
		}
		auto iter = this->mSessions.find(id);
		if(iter != this->mSessions.end())
		{
			delete message;
			iter->second->StartWrite(wsMessage.release());
			return XCode::Ok;
		}
		auto iter1 = this->mClients.find(id);
		if(iter1 != this->mClients.end())
		{
			delete message;
			iter1->second->StartWrite(wsMessage.release());
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
		requestClient->StartWrite(wsMessage.release());
		return XCode::SendMessageFail;
	}

	void WebSocketComponent::OnClientError(int id, int code)
	{
		auto iter = this->mSessions.find(id);
		if(iter != this->mSessions.end())
		{
			this->mSessions.erase(iter);
			LOG_WARN("remove ws client id:{} code=>{}", id, code);
		}
	}
}