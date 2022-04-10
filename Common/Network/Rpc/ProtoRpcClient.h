#pragma once
#include"RpcClient.h"
#include"Protocol/com.pb.h"
#include"Async/TaskSource.h"
#include<google/protobuf/message.h>
using namespace google::protobuf;

namespace Sentry
{
	class RpcClientComponent;
	class ProtoRpcClient : public RpcClient
	{
	 public:
		explicit ProtoRpcClient(RpcClientComponent* component, std::shared_ptr<SocketProxy> socket, SocketType type);
		~ProtoRpcClient() override = default;
	 public:
		void StartClose();
		void SendToServer(std::shared_ptr<com::Rpc_Request> message);
		void SendToServer(std::shared_ptr<com::Rpc_Response> message);
		std::shared_ptr<TaskSource<bool>> ConnectAsync(const std::string& ip, unsigned short port);
	 protected:
		void OnConnect(XCode code) final;
		void OnClientError(XCode code) final;
		XCode OnRequest(const char* buffer, size_t size) final;
		XCode OnResponse(const char* buffer, size_t size) final;
		void OnSendData(XCode code, std::shared_ptr<NetworkData> message) final;
	 private:
		int mConnectCount;
		RpcClientComponent* mTcpComponent;
		std::vector<std::shared_ptr<TaskSource<bool>>> mConnectTasks;
	};
}// namespace Sentry