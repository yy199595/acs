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
		long long GetLastOperTime() const { return this->mLastOperTime;}
		const std::string & GetAddress() { return this->mSocket->GetAddress();}
	 protected:
		void SetBufferCount(int count, int maxCount);
		virtual void OnConnect(const asio::error_code & error)
		{
			throw std::logic_error("%%%%%%%%%%%%%%%");
		}
		virtual void OnSendMessage(const asio::error_code & code, std::shared_ptr<ProtoMessage> message) = 0;
	protected:
		void Connect();
		void ReceiveHead(int size);
		void ReceiveBody(int size);
        void Send(std::shared_ptr<ProtoMessage> message);
        virtual void OnReceiveHead(const asio::error_code & code, const char * message, size_t size) {}
        virtual void OnReceiveBody(const asio::error_code & code, const char * message, size_t size) {}
	 protected:
		IAsioThread& mNetworkThread;
		std::shared_ptr<SocketProxy> mSocket;
	 private:
		int mBufferCount;
		char * mRecvBuffer;
		int mBufferMaxCount;
		long long mLastOperTime;
		asio::streambuf mSendBuffer;
	};
}
#endif //GAMEKEEPER_TCPCLIENT_H
