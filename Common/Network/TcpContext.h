//
// Created by zmhy0073 on 2022/1/15.
//

#ifndef GAMEKEEPER_TCPCLIENT_H
#define GAMEKEEPER_TCPCLIENT_H
#include"XCode/XCode.h"
#include"SocketProxy.h"
#include"Proto/ProtoMessage.h"
#define TCP_RECEIVE_MAX_SIZE 1024 * 20
using namespace Sentry;
namespace Tcp
{
 	class TcpContext : public std::enable_shared_from_this<TcpContext>
	{
	 public:
		TcpContext(std::shared_ptr<SocketProxy> socket);
		virtual ~TcpContext();
	 public:
		bool IsOpen() { return this->mSocket->IsOpen();}
		const std::string & GetAddress() { return this->mSocket->GetAddress();}
	 protected:
		void StartReceive();
		void StartConnect();
		void SendProtoMessage(std::shared_ptr<ProtoMessage> message);
	 protected:
		virtual size_t GetRecvSize() = 0;
		virtual void OnError(const asio::error_code & error) = 0;
		virtual void OnConnect(const asio::error_code & error) = 0;
		virtual bool OnRecvMessage(const char * message, size_t size) = 0;
	 private:
		void Connect();
		void ReceiveHead();
		void ReceiveBody(int size);
		void Send(std::shared_ptr<ProtoMessage> message);
	 protected:
		AsioContext& mContext;
		IAsioThread& mNetworkThread;
		std::shared_ptr<SocketProxy> mSocket;
	 private:
		char * mRecvBuffer;
		size_t mRecvMaxSize;
		asio::streambuf mSendBuffer;
	};
}
#endif //GAMEKEEPER_TCPCLIENT_H
