#pragma once
#include"XCode/XCode.h"
#include"Message/c2s.pb.h"
#include"Async/TaskSource.h"
#include"Network/TcpContext.h"


using namespace Sentry;
using namespace Tcp;
namespace Client
{
	class ClientComponent;
	class TcpRpcClientContext : public Tcp::TcpContext
	{
	public:
		TcpRpcClientContext(std::shared_ptr<SocketProxy> socket, ClientComponent * component);
	public:
		bool StartConnect();
		void StartReceive();
		void SendToServer(std::shared_ptr<c2s::Rpc::Request> request);
	protected:
		bool OnRequest(std::iostream & os);
		bool OnResponse(std::iostream & os);
        void OnReceiveMessage(const asio::error_code &code, asio::streambuf &buffer) final;
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
    private:
        ClientComponent * mClientComponent;
        std::shared_ptr<SocketProxy> mTcpSocket;
        std::shared_ptr<TaskSource<bool>> mConnectTask;
    };
}