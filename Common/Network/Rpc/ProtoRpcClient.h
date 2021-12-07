#pragma once
#include"RpcClient.h"
#include<google/protobuf/message.h>
using namespace google::protobuf;

namespace GameKeeper
{
	class ProtoRpcClientComponent;
	class ProtoRpcClient : public RpcClient
	{
	public:
		explicit ProtoRpcClient(ProtoRpcClientComponent *component, SocketProxy * socket, SocketType type);
		~ProtoRpcClient() override = default;
	public:
		void StartClose();
		bool StartSendProtocol(char type, const Message * message);

	protected:
        void OnClose(XCode code) final;
        void OnConnect(XCode code) final;
		XCode OnRequest(const char * buffer, size_t size)final;
		XCode OnResponse(const char * buffer, size_t size)final;
	private:
		void SendProtocol(char type, const Message * message);
    private:

        ProtoRpcClientComponent * mTcpComponent;
		std::queue<const Message *> mMessageQueue;
	};
}// namespace GameKeeper