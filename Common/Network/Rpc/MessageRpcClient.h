#pragma once
#include"Network/TcpContext.h"
#include"Async/TaskSource.h"
#include"Coroutine/CoroutineLock.h"
#include<google/protobuf/message.h>

using namespace Tcp;
using namespace google::protobuf;

namespace Sentry
{
	class RpcClientComponent;
	class MessageRpcClient : public Tcp::TcpContext
	{
	 public:
		explicit MessageRpcClient(RpcClientComponent* component, std::shared_ptr<SocketProxy> socket);
		~MessageRpcClient() override = default;
	 public:
		void StartClose();
		void StartReceive();
		void SendToServer(std::shared_ptr<com::rpc::request> message);
		void SendToServer(std::shared_ptr<com::rpc::response> message);
	 protected:
		void OnConnect(const asio::error_code &error, int count) final;
        void OnReceiveLength(const asio::error_code &code, int length) final;
        void OnReceiveMessage(const asio::error_code &code, std::istream & is, size_t) final;
        void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
    private:
		void CloseSocket(XCode code);
		bool OnRequest(std::istream & istream1);
		bool OnResponse(std::istream & istream1);
	private:
		RpcClientComponent* mTcpComponent;
		std::shared_ptr<asio::steady_timer> mTimer;
	};
}// namespace Sentry