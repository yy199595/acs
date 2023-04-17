//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_RPCPROXYCLIENT_H
#define GAMEKEEPER_RPCPROXYCLIENT_H
#include"Rpc/Client/Message.h"
#include"Network/Tcp/TcpContext.h"
using namespace Tcp;
namespace Tendo
{
    // 网关session
	class OuterNetComponent;
 	class OuterNetTcpClient : public Tcp::TcpContext
	{
	 public:
		OuterNetTcpClient(std::shared_ptr<Tcp::SocketProxy> socket, OuterNetComponent* component);
		~OuterNetTcpClient() final = default;
	 public:
		void StartClose();
		void StartReceive(int second = 0);
		void SendData(std::shared_ptr<Msg::Packet> message);
	 protected:
		void OnTimeOut() final;
        void OnReceiveMessage(const asio::error_code &code, std::istream & readStream, size_t) final;
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
	private:
        void CloseSocket(int code);
	 private:
		unsigned int mMaxQps;
        Tcp::DecodeState mState;
        OuterNetComponent* mGateComponent;
        std::shared_ptr<Msg::Packet> mMessage;
	};
}


#endif //GAMEKEEPER_RPCPROXYCLIENT_H
