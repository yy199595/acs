#pragma once
#include"XCode/XCode.h"
#include"Protocol/c2s.pb.h"
#include"Async/TaskSource.h"
#include"Network/Rpc/RpcClient.h"


using namespace Sentry;
namespace Client
{
	class ClientComponent;
	class TcpRpcClient
	{
	public:
		TcpRpcClient(ClientComponent * component);
	public:
		bool StartSendProtoData(std::shared_ptr<c2s::Rpc_Request> request);
        std::shared_ptr<TaskSource<bool>> ConnectAsync(const std::string & ip, unsigned short port);
	protected:
        XCode OnRequest(const char * buffer, size_t size);
		XCode OnResponse(const char * buffer, size_t size);
	private:
		void SendProtoData(const c2s::Rpc_Request * request);
	private:
        char mRecvBuffer[4096];
        ClientComponent * mClientComponent;
        std::shared_ptr<AsioTcpSocket> mTcpSocket;
    };
}