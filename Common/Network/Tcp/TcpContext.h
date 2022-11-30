//
// Created by zmhy0073 on 2022/1/15.
//

#ifndef GAMEKEEPER_TCPCLIENT_H
#define GAMEKEEPER_TCPCLIENT_H
#include<list>
#include"XCode/XCode.h"
#include"SocketProxy.h"
#include"Message/ProtoMessage.h"
using namespace Sentry;

namespace Tcp
{
    enum class DecodeState
    {
        Head,
        Body
    };

 	class TcpContext : public std::enable_shared_from_this<TcpContext>
	{
	 public:
		TcpContext(std::shared_ptr<SocketProxy> socket, size_t maxCount = 10240);
		virtual ~TcpContext();

	public:
        bool Reset(std::shared_ptr<SocketProxy> socket);
        size_t WaitSendCount() const { return this->mSendCount; }
        long long GetLastOperTime() const { return this->mLastOperTime;}
		const std::string & GetAddress() { return this->mSocket->GetAddress();}
        std::shared_ptr<SocketProxy> MoveSocket() { return std::move(this->mSocket); }
	protected:
		void Connect();
		void ReceiveLine();
		void ReceiveSomeMessage();
		void ReceiveMessage(int size);
        void Write(std::shared_ptr<ProtoMessage> message);
		template<typename T>
		std::shared_ptr<T> Cast() { return std::dynamic_pointer_cast<T>(this->shared_from_this());}
	 protected:
		bool ConnectSync(); //同步连接
		int RecvLineSync(); //同步读一行
		int RecvSync(int read); //同步读取数据
		void ClearSendStream();
		void ClearRecvStream();
        bool SendFromMessageQueue();
        int SendSync(std::shared_ptr<ProtoMessage> message); //同步发送
    protected:
        size_t PopAllMessage();
        std::shared_ptr<ProtoMessage> PopMessage();
	 protected:
        virtual void OnConnect(const Asio::Code & error, int count) { throw std::logic_error("");}
        virtual void OnReceiveLine(const Asio::Code & code, std::istream & readStream, size_t size) {}
        virtual void OnReceiveMessage(const Asio::Code & code, std::istream & readStream, size_t size) {}
		virtual void OnSendMessage(const Asio::Code & code, std::shared_ptr<ProtoMessage> message) {  };
	 protected:
		int mConnectCount;
        asio::streambuf mSendBuffer;
        asio::streambuf mRecvBuffer;
		std::shared_ptr<SocketProxy> mSocket;
	 private:
        size_t mSendCount;   
		const size_t mMaxCount;
		long long mLastOperTime;
        std::list<std::shared_ptr<ProtoMessage>> mMessagqQueue;
	};
}
#endif //GAMEKEEPER_TCPCLIENT_H
