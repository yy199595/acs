#pragma once
#include"Network/Rpc.h"
#include"Network/TcpContext.h"
#include"Async/TaskSource.h"
#include"Coroutine/CoroutineLock.h"
#include<google/protobuf/message.h>

using namespace Tcp;
using namespace google::protobuf;

namespace Sentry
{
	class InnerNetComponent;
	class InnerNetClient : public Tcp::TcpContext
	{
	 public:
		explicit InnerNetClient(InnerNetComponent* component, std::shared_ptr<SocketProxy> socket);
		~InnerNetClient() override = default;
	 public:
		void StartClose();
		void StartReceive();
		void SendToServer(std::shared_ptr<com::rpc::request> message);
		void SendToServer(std::shared_ptr<com::rpc::response> message);
    private:
        void CloseSocket(XCode code);
        void OnConnect(const asio::error_code &error, int count) final;
        void OnReceiveMessage(const asio::error_code &code, std::istream & is, size_t) final;
        void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
	private:
        Tcp::DecodeState mState;
		InnerNetComponent* mTcpComponent;
        std::shared_ptr<Tcp::BinMessage> mMessage;
        std::shared_ptr<asio::steady_timer> mTimer;
	};
}// namespace Sentry