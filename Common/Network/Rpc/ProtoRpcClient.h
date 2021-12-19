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
		explicit ProtoRpcClient(RpcClientComponent *component, SocketProxy * socket, SocketType type);
		~ProtoRpcClient() override = default;
	public:
		void StartClose();
        bool SendToServer(const com::Rpc_Request * message);
        bool SendToServer(const com::Rpc_Response * message);
	protected:
        void OnClose(XCode code) final;
        void OnConnect(XCode code) final;
        void OnSendData(XCode code, const Message *) final;
		XCode OnRequest(const char * buffer, size_t size)final;
		XCode OnResponse(const char * buffer, size_t size)final;
    private:

        RpcClientComponent * mTcpComponent;
		std::queue<const Message *> mMessageQueue;
	};
}// namespace GameKeeper