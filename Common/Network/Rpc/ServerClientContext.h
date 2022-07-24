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
	class ServerClientContext : public Tcp::TcpContext
	{
	 public:
		explicit ServerClientContext(RpcClientComponent* component, std::shared_ptr<SocketProxy> socket);
		~ServerClientContext() override = default;
	 public:
		void StartClose();
		void StartReceive();
		void SendToServer(std::shared_ptr<com::Rpc_Request> message);
		void SendToServer(std::shared_ptr<com::Rpc_Response> message);
	 protected:
		void OnConnect(const asio::error_code &error, int count) final;
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
        void OnReceiveMessage(const asio::error_code &code, asio::streambuf &buffer, size_t size) final;
    private:
		void CloseSocket(XCode code);
		bool OnRequest(const char * buffer, size_t size);
		bool OnResponse(const char * buffer, size_t size);
	private:
        char mDataBuffer[1024];
		RpcClientComponent* mTcpComponent;
		std::shared_ptr<asio::steady_timer> mTimer;
		std::queue<std::shared_ptr<ProtoMessage>> mSendQueues;
	};
}// namespace Sentry