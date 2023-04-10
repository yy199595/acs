//
// Created by zmhy0073 on 2022/1/15.
//

#ifndef GAMEKEEPER_TCPCLIENT_H
#define GAMEKEEPER_TCPCLIENT_H
#include<list>
#include"SocketProxy.h"

namespace Tcp
{
    enum class DecodeState
    {
        Head,
        Body
    };
    class ProtoMessage;
 	class TcpContext : public std::enable_shared_from_this<TcpContext>
	{
	 public:
		explicit TcpContext(std::shared_ptr<SocketProxy> socket, size_t maxCount = 10240);
		virtual ~TcpContext() = default;

	public:
        bool Reset(std::shared_ptr<SocketProxy> socket);
        size_t WaitSendCount() const { return this->mSendCount; }
        long long GetLastOperTime() const { return this->mLastOperTime;}
		const std::string & GetAddress() { return this->mSocket->GetAddress();}
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
		size_t RecvLineSync(); //同步读一行
		size_t RecvSync(int read); //同步读取数据
		void ClearSendStream();
		void ClearRecvStream();
        bool SendFromMessageQueue();
        size_t SendSync(const std::shared_ptr<ProtoMessage>& message); //同步发送
    protected:
        size_t PopAllMessage();
		std::shared_ptr<ProtoMessage> PopMessage();
	protected:
		void StopTimer();
		void StartTimer(int timeout);
	 protected:
		virtual void OnTimeOut() { }
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
		std::unique_ptr<asio::steady_timer> mTimer;
        std::list<std::shared_ptr<ProtoMessage>> mMessageQueue;
	};
}

#endif //GAMEKEEPER_TCPCLIENT_H
