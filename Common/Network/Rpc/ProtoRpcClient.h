#pragma once
#include"RpcClient.h"


namespace GameKeeper
{
	class ProtoRpcComponent;
	class ProtoRpcClient : public RpcClient
	{
	public:
		explicit ProtoRpcClient(ProtoRpcComponent *component, SocketProxy * socket, SocketType type);
		~ProtoRpcClient() override = default;
	public:
		void StartClose();
		bool StartSendProtocol(char type, const Message * message);

	protected:
        void OnConnect(XCode code) final;
        void CloseSocket(XCode code) final;
		bool OnRequest(const char * buffer, size_t size)final;
		bool OnResponse(const char * buffer, size_t size)final;
	private:
		void SendProtocol(char type, const Message * message);
    private:

        ProtoRpcComponent * mTcpComponent;
		std::queue<const Message *> mMessageQueue;
	};
}// namespace GameKeeper