//
// Created by 64658 on 2025/1/2.
//

#ifndef APP_INNERWEBSOCKETCOMPONENT_H
#define APP_INNERWEBSOCKETCOMPONENT_H
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
	class InnerWebSocketComponent : public ITcpListen, public rpc::IInnerSender,
									public RpcComponent<rpc::Message>, public IRpc<rpc::Message, rpc::Message>
	{
	public:
		InnerWebSocketComponent();
	public:
		bool LateAwake() final;
		void OnClientError(int id, int code) final;
		bool OnListen(tcp::Socket *socket) noexcept final;
		inline char GetNet() const noexcept final { return rpc::net::ws; }
		void OnMessage(rpc::Message *request, rpc::Message *response) noexcept final;
	public:
		int Create(const std::string & address);
		int Send(int id, std::unique_ptr<ws::Message>& message) noexcept;
		int Send(int id, std::unique_ptr<rpc::Message> &message) noexcept final;
	private:
		class NodeComponent * mActor;
		class ThreadComponent * mThread;
		math::NumberPool<int> mClientPool;
		class DispatchComponent * mDispatch;
		std::unordered_map<int, std::shared_ptr<ws::Client>> mClients;
		std::unordered_map<int, std::shared_ptr<ws::Session>> mSessions;
	};
}


#endif //APP_INNERWEBSOCKETCOMPONENT_H
