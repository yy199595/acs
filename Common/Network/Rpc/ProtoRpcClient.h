#pragma once
#include"RpcClient.h"
#include"Protocol/com.pb.h"
#include<google/protobuf/message.h>
using namespace google::protobuf;

namespace GameKeeper
{
	class RpcClientComponent;
	class ProtoRpcClient : public RpcClient
	{
	public:
		explicit ProtoRpcClient(RpcClientComponent *component, std::shared_ptr<SocketProxy> socket, SocketType type);
		~ProtoRpcClient() override = default;
	public:
		void StartClose();
        void SendToServer(std::shared_ptr<com::Rpc_Request> message);
        void SendToServer(std::shared_ptr<com::Rpc_Response> message);
	protected:
        void OnClose(XCode code) final;
        void OnConnect(XCode code) final;
		XCode OnRequest(const char * buffer, size_t size)final;
		XCode OnResponse(const char * buffer, size_t size)final;
        void OnSendData(XCode code, std::shared_ptr<Message> ) final;

    private:
        bool SendFromQueue();
        bool ConnectInSecond(int second);
        void Send(char type, std::shared_ptr<Message> message);
    private:
        int mConnectCount;
        asio::steady_timer * mConnectTimer;
        RpcClientComponent * mTcpComponent;
		std::queue<std::shared_ptr<Message>> mMessageQueue;
	};
}// namespace GameKeeper