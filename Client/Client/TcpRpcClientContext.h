#pragma once
#include"XCode/XCode.h"
#include"Client/Message.h"
#include"Tcp/TcpContext.h"
#include"Message/c2s.pb.h"
#include"Source/TaskSource.h"


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
		void SendToServer(std::shared_ptr<Rpc::Packet> request);
	protected:
        void OnReceiveMessage(const asio::error_code &code, std::istream & readStream, size_t) final;
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
    private:
        Tcp::DecodeState mState;
        ClientComponent * mClientComponent;
        std::shared_ptr<Rpc::Packet> mMessage;
    };
}