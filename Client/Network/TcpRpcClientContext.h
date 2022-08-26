#pragma once
#include"Network/Rpc.h"
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
		void SendToServer(std::shared_ptr<c2s::rpc::request> request);
	protected:
        void OnReceiveMessage(const asio::error_code &code, std::istream & readStream, size_t) final;
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
    private:
        char mDataBuffer[1024];
        Tcp::DecodeState mState;
        ClientComponent * mClientComponent;
        std::shared_ptr<Tcp::RpcMessage> mMessage;
    };
}