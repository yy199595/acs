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


	class ReadStreamHelper
	{
	public:
		ReadStreamHelper(std::istream & ss)
			: mStream(ss) {}

	public:
		inline std::string ReadString()
		{
			std::string ret;
			char cc = this->mStream.get();
			while(cc != '\0')
			{
				ret += cc;
				cc = this->mStream.get();
			}
			return ret;
		}
		inline std::string ReadString(int size)
		{
			std::unique_ptr<char[]> buffer(new char[size]);
			this->mStream.readsome(buffer.get(), size);
			return std::string(buffer.get(), size);
		}

		template<typename T>
		inline T ReadByType()
		{
			T data;
			this->mStream.readsome((char *)&data, sizeof(T));
			return data;
		}
	private:
		std::istream & mStream;
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
        void ReceiveLength();
		void ReceiveSomeMessage();
		void ReceiveMessage(int size);
        void Send(std::shared_ptr<ProtoMessage> message);
		template<typename T>
		std::shared_ptr<T> Cast() { return dynamic_pointer_cast<T>(this->shared_from_this());}
	 protected:
		bool ConnectSync(); //同步连接
		int RecvLineSync(); //同步读一行
		int RecvSync(int read); //同步读取数据
		void ClearSendStream();
		void ClearRecvStream();
        void SendFromMessageQueue();
        int SendSync(std::shared_ptr<ProtoMessage> message); //同步发送
    protected:
        size_t PopAllMessage();
        std::shared_ptr<ProtoMessage> PopMessage();
	 protected:
        virtual void OnReceiveLength(const asio::error_code & code, int length) { }
        virtual void OnReceiveLine(const asio::error_code & code, std::istream & readStream) {}
        virtual void OnReceiveMessage(const asio::error_code & code, std::istream & readStream) {}
        virtual void OnConnect(const asio::error_code & error, int count) { throw std::logic_error("");}
		virtual void OnSendMessage(const asio::error_code & code, std::shared_ptr<ProtoMessage> message) {  };
	 protected:
        asio::streambuf mSendBuffer;
        asio::streambuf mRecvBuffer;
		std::shared_ptr<SocketProxy> mSocket;
	 private:
        size_t mSendCount;
        int mConnectCount;
		const size_t mMaxCount;
		long long mLastOperTime;
        std::list<std::shared_ptr<ProtoMessage>> mMessagqQueue;
	};
}
#endif //GAMEKEEPER_TCPCLIENT_H
