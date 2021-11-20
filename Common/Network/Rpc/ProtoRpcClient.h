#pragma once
#include"RpcClient.h"


namespace GameKeeper
{
	class ProtoRpcComponent;
	class ProtoRpcClient : public RpcClient
	{
	public:
		explicit ProtoRpcClient(ProtoRpcComponent *component, SocketProxy * socket);
		virtual ~ProtoRpcClient() override = default;
	public:
		void StartClose();
		void StartSendProtocol(char type, const Message * message);
		virtual SocketType GetSocketType() { return SocketType::RemoteSocket; }
	protected:
		void CloseSocket(XCode code) final;
		bool OnRequest(char * buffer, size_t size)final;
		bool OnResponse(char * buffer, size_t size)final;
	private:
		void SendProtocol(char type, const Message * message);
	protected:
		ProtoRpcComponent * mTcpComponent;
		std::queue<const Message *> mMessageQueue;
	};
}// namespace GameKeeper