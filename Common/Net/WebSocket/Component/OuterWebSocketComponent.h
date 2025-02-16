//
// Created by 64658 on 2025/1/2.
//

#ifndef APP_OuterWsComponent_H
#define APP_OuterWsComponent_H
namespace ws
{
	class Message;
	class Session;
	class Client;
}

#include "Rpc/Component/RpcComponent.h"
#include "Server/Component/ITcpComponent.h"

namespace acs
{
	class OuterWebSocketComponent final : public ITcpListen, public rpc::IOuterSender, public IServerRecord,
							 public RpcComponent<rpc::Message>, public IRpc<rpc::Message, rpc::Message>
	{
	public:
		OuterWebSocketComponent();
	public:
		bool LateAwake() final;
		void OnClientError(int id, int code) final;
		bool OnListen(tcp::Socket *socket) noexcept final;
		void OnMessage(int, rpc::Message *request, rpc::Message *response) noexcept final;
	public:
		void Broadcast(rpc::Message *message) noexcept final;
		int Send(int id, rpc::Message *message) noexcept final;
		char GetNet() const noexcept final { return rpc::Net::Ws; }
	private:
		void OnRecord(json::w::Document &document) final;
		void OnPlayerLogin(long long userId, int sockId);
		void OnPlayerLogout(long long userId, int sockId);
	private:
		unsigned int mSumCount;
		class GateComponent * mGate;
		math::NumberPool<int> mClientPool;
		std::unordered_map<int, std::shared_ptr<ws::Session>> mSessions;
	};
}


#endif //APP_OuterWsComponent_H
