#pragma once
#include"XCode/XCode.h"
#include"Protocol/c2s.pb.h"
#include"Network/Rpc/RpcClient.h"


using namespace GameKeeper;
namespace Client
{
	class ClientComponent;
	class TcpRpcClient : public RpcClient
	{
	public:
		TcpRpcClient(SocketProxy * socket, ClientComponent * component);
	public:
		bool StartSendProtoData(const c2s::Rpc_Request * request);
		bool AwaitConnect(const std::string & ip, unsigned short port);
	protected:
        void OnClose(XCode code) final;
		void OnConnect(XCode code) final { }
        void OnSendData(XCode code, const Message *) final;
        XCode OnRequest(const char * buffer, size_t size) final;
		XCode OnResponse(const char * buffer, size_t size) final;
	private:
		void SendProtoData(const c2s::Rpc_Request * request);
	private:
        bool mIsConnectSuccessful;
		unsigned int mCoroutineId;
		ClientComponent * mClientComponent;

	};
}