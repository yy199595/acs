//
// Created by zmhy0073 on 2022/1/15.
//

#ifndef APP_TCPCLIENT_H
#define APP_TCPCLIENT_H
#include"Socket.h"
#include"Core/Queue/Queue.h"

namespace tcp
{
	enum TimeoutFlag
	{
		ReadLine = 1,
		ReadSome = 2,
		ReadCount = 3,
		Connect = 4,
		Write = 5
	};
}

namespace tcp
{
    class IProto;
 	class TcpClient
	{
	 public:
		explicit TcpClient(size_t maxCount);
		explicit TcpClient(tcp::Socket *socket, size_t maxCount);
		virtual ~TcpClient() = default;
	public:
		void SetSocket(Socket * socket);
		const std::string & GetAddress() { return this->mSocket->GetAddress();}
	protected:
		void ClearBuffer();
		void ReadAll(int timeout = 0);
		void Connect(int timeout = 0);
		void ReadLine(int timeout = 0);
		void ReadSome(int timeout = 0);
		void ReadLength(int size, int timeout = 0);
		void Connect(const std::string & host, const std::string & port, int timeout = 0);
	protected:
		bool SendSync(tcp::IProto & message); //同步发送
		void Write(tcp::IProto & message, int timeout = 0);
	protected:
		void ClearSendStream();
		void ClearRecvStream();
		bool ConnectSync(Asio::Code & code);
		bool RecvLineSync(size_t & size); //同步读一行
		bool RecvSync(int read, size_t & size); //同步读取数据
		bool ConnectSync(const std::string & host, const std::string & port);

	protected:
		void StopTimer();
		void StopUpdate();
		void StartUpdate(int timeout);
		void StartTimer(int timeout, TimeoutFlag flag);
	protected:
		virtual void OnUpdate() { }
		virtual void OnTimeout(TimeoutFlag flag) { }
		virtual void OnReadError(const Asio::Code & code) = 0;
		virtual void OnConnect(bool result, int count) { throw std::logic_error("");}
        virtual void OnReceiveLine(std::istream & readStream, size_t size) {}
        virtual void OnReceiveMessage(std::istream & readStream, size_t size) {}
	protected:
		virtual void OnSendMessage() { }
		virtual void OnSendMessage(const Asio::Code & code) {  };
	protected:
		const size_t mMaxCount;
		asio::streambuf mSendBuffer;
        asio::streambuf mRecvBuffer;
		std::unique_ptr<tcp::Socket> mSocket;
	private:
		int mConnectCount;
		std::unique_ptr<asio::steady_timer> mTimer;
		std::unique_ptr<asio::steady_timer> mUpdateTimer;
	};
}

#endif //APP_TCPCLIENT_H
