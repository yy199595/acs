//
// Created by zmhy0073 on 2022/1/15.
//

#ifndef GAMEKEEPER_TCPCLIENT_H
#define GAMEKEEPER_TCPCLIENT_H
#include"XCode/XCode.h"
#include"SocketProxy.h"
#include"Proto/ProtoMessage.h"
using namespace Sentry;

namespace Tcp
{
    enum class ReadType
    {
        HEAD,
        BODY,
        LINE
    };

 	class TcpContext : public std::enable_shared_from_this<TcpContext>
	{
	 public:
		TcpContext(std::shared_ptr<SocketProxy> socket, size_t maxCount = 10240);
		virtual ~TcpContext();

	public:
		bool IsOpen() { return this->mSocket->IsOpen();}
		long long GetLastOperTime() const { return this->mLastOperTime;}
		const std::string & GetAddress() { return this->mSocket->GetAddress();}
	 protected:
		virtual void OnConnect(const asio::error_code & error)
		{
			throw std::logic_error("%%%%%%%%%%%%%%%");
		}
		virtual void OnSendMessage(const asio::error_code & code, std::shared_ptr<ProtoMessage> message) = 0;
	protected:
		void Connect();
        void ReceiveLine();
		void ReceiveMessage(int size);
        int GetLength(const std::string & buffer);
        void Send(std::shared_ptr<ProtoMessage> message);
        virtual void OnReceiveLine(const asio::error_code & code, const std::string & buffer) {}
        virtual void OnReceiveMessage(const asio::error_code & code, const std::string & buffer) {}
	 protected:
        ReadType mReadState;
        IAsioThread& mNetworkThread;
		std::shared_ptr<SocketProxy> mSocket;
	 private:
        const size_t mMaxCount;
		long long mLastOperTime;
        std::string mRecvBuffer;
		asio::streambuf mSendBuffer;
	};
}
#endif //GAMEKEEPER_TCPCLIENT_H
