//
// Created by 64658 on 2025/1/2.
//

#ifndef APP_INNERWEBSOCKETCOMPONENT_H
#define APP_INNERWEBSOCKETCOMPONENT_H
namespace ws
{
	class Message;
	class SessionClient;
	class RequestClient;
}

#include "Rpc/Component/RpcComponent.h"
#include "Server/Component/ITcpComponent.h"
#include "WebSocket/Common/WebSocketMessage.h"
namespace acs
{
	class InnerWebSocketComponent : public ITcpListen, public rpc::IInnerSender, public IComplete,
									public RpcComponent<rpc::Message>, public IRpc<rpc::Message, rpc::Message>
	{
	public:
		InnerWebSocketComponent();
	public:
		void Complete() final;
		bool LateAwake() final;
		bool OnListen(tcp::Socket *socket) final;
		void OnClientError(int id, int code) final;
		void OnMessage(int, rpc::Message *request, rpc::Message *response) final;
	public:
		int Send(int id, rpc::Message *message) final;
		char GetNet() const final { return rpc::Net::Ws; }
	private:
		class ActorComponent * mActor;
		class ThreadComponent * mThread;
		math::NumberPool<int> mClientPool;
		class DispatchComponent * mDispatch;
		std::unordered_map<int, std::shared_ptr<ws::RequestClient>> mClients;
		std::unordered_map<int, std::shared_ptr<ws::SessionClient>> mSessions;
	};
}


#endif //APP_INNERWEBSOCKETCOMPONENT_H
