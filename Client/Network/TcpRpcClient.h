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
		void OnConnect(XCode code) { }
		void CloseSocket(XCode code);
		XCode OnRequest(const char * buffer, size_t size);
		XCode OnResponse(const char * buffer, size_t size);
	private:
		void ConnectHandler(XCode code);
		void SendProtoData(const c2s::Rpc_Request * request);
	private:
		std::string mAddress;
		unsigned int mCoroutineId;
		ClientComponent * mClientComponent;

	};
}