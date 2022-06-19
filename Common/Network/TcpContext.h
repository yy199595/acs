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
		long long GetLastOperTime() const { return this->mLastOperTime;}
		const std::string & GetAddress() { return this->mSocket->GetAddress();}
	protected:
		void Connect();
        void ReceiveLine();
		void ReceiveSomeMessage();
		void ReceiveMessage(int size);
        int GetLength(asio::streambuf & buffer);
        void Send(std::shared_ptr<ProtoMessage> message);
		template<typename T>
		std::shared_ptr<T> Cast() { return dynamic_pointer_cast<T>(this->shared_from_this());}
	 protected:
		virtual void OnConnect(const asio::error_code & error, int count) { throw std::logic_error("");}
		virtual void OnReceiveLine(const asio::error_code & code, asio::streambuf & buffer) {}
        virtual void OnReceiveMessage(const asio::error_code & code, asio::streambuf & buffer) {}
		virtual void OnSendMessage(const asio::error_code & code, std::shared_ptr<ProtoMessage> message) { };
	 protected:
        ReadType mReadState;
        IAsioThread& mNetworkThread;
		std::shared_ptr<SocketProxy> mSocket;
	 private:
		int mConnectCount;
		const size_t mMaxCount;
		long long mLastOperTime;
		asio::streambuf mSendBuffer;
		asio::streambuf mRecvBuffer;
	};
}
#endif //GAMEKEEPER_TCPCLIENT_H
