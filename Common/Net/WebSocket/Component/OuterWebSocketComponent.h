//
// Created by 64658 on 2025/1/2.
//

#ifndef APP_OuterWsComponent_H
#define APP_OuterWsComponent_H
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
	class OuterWebSocketComponent : public ITcpListen, public rpc::IOuterSender,
							 public RpcComponent<rpc::Message>, public IRpc<rpc::Message, rpc::Message>
	{
	public:
		OuterWebSocketComponent();
	public:
		bool LateAwake() final;
		bool OnListen(tcp::Socket *socket) final;
		void OnClientError(int id, int code) final;
		void OnMessage(int, rpc::Message *request, rpc::Message *response) final;
	public:
		void Broadcast(rpc::Message *message) final;
		int Send(int id, rpc::Message *message) final;
		char GetNet() const final { return rpc::Net::Ws; }
	private:
		class GateComponent * mGate;
		math::NumberPool<int> mClientPool;
		std::unordered_map<int, std::shared_ptr<ws::SessionClient>> mSessions;
	};
}


#endif //APP_OuterWsComponent_H
