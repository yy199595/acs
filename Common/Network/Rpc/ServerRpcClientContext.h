#pragma once
#include"Network/TcpContext.h"
#include"Protocol/com.pb.h"
#include"Async/TaskSource.h"
#include"Coroutine/CoroutineLock.h"
#include<google/protobuf/message.h>

using namespace Tcp;
using namespace google::protobuf;

namespace Sentry
{
	class RpcClientComponent;
	class ServerRpcClientContext : public Tcp::TcpContext
	{
	 public:
		explicit ServerRpcClientContext(RpcClientComponent* component, std::shared_ptr<SocketProxy> socket);
		~ServerRpcClientContext() override = default;
	 public:
		void StartClose();
		void StartReceive();
		bool StartConnectAsync();
		void SendToServer(std::shared_ptr<com::Rpc_Request> message);
		void SendToServer(std::shared_ptr<com::Rpc_Response> message);
	 protected:
		void OnConnect(const asio::error_code &error) final;
		bool OnRecvMessage(const asio::error_code &code, const char *message, size_t size) final;
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;

	private:
		void CloseSocket(XCode code);
		bool OnRequest(const char * message, size_t size);
		bool OnResponse(const char * message, size_t size);
	private:
		int mConnectCount;
		RpcClientComponent* mTcpComponent;
		std::shared_ptr<CoroutineLock> mConnectLock;
		std::shared_ptr<TaskSource<XCode>> mConnectTask;
	};
}// namespace Sentry