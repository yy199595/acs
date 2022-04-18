#pragma once
#include"XCode/XCode.h"
#include"Protocol/c2s.pb.h"
#include"Async/TaskSource.h"
#include"Network/Rpc/RpcClient.h"


using namespace Sentry;
namespace Client
{
	class ClientComponent;
    class TcpRpcClient : public RpcClient
	{
	public:
		TcpRpcClient(std::shared_ptr<SocketProxy> socket, ClientComponent * component);
	public:
		std::shared_ptr<TaskSource<bool>> ConnectAsync();
		std::shared_ptr<TaskSource<bool>> SendToGate(std::shared_ptr<c2s::Rpc_Request> request);
	protected:
        void OnConnect(XCode code) final;
        void OnClientError(XCode code) final;
        XCode OnRequest(const char * buffer, size_t size) final;
		XCode OnResponse(const char * buffer, size_t size) final;
        void OnSendData(XCode code, std::shared_ptr<NetworkData> message) final;
    private:
        char mRecvBuffer[4096];
        ClientComponent * mClientComponent;
        std::shared_ptr<SocketProxy> mTcpSocket;
        std::shared_ptr<TaskSource<bool>> mSendTask;
        std::shared_ptr<TaskSource<bool>> mConnectTask;
    };
}