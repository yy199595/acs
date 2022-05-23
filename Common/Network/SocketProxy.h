#pragma once
#include<mutex>
#include<Define/CommonLogDef.h>
#include<Thread/TaskThread.h>

namespace Sentry
{
	class SocketProxy
	{
	 public:
		SocketProxy(IAsioThread& thread);
		SocketProxy(IAsioThread& thread, std::shared_ptr<AsioTcpSocket> socket);
		SocketProxy(IAsioThread& thread, const std::string & ip, unsigned short port);
		~SocketProxy() = default;
	 public:
		void Close();
		unsigned short GetPort() { return this->mPort;}
		inline std::string & GetIp() { return this->mIp;}
		inline AsioTcpSocket& GetSocket() { return *mSocket;}
		inline bool IsOpen() { return this->mSocket->is_open(); }
		inline IAsioThread& GetThread() { return this->mNetThread; }
		inline const std::string& GetAddress() { return this->mAddress; }
		//inline AsioContext& GetContext() { return this->mNetThread.GetContext(); }
	 private:
		std::string mIp;
		unsigned short mPort;
		std::string mAddress;
		IAsioThread& mNetThread;
		std::shared_ptr<AsioTcpSocket> mSocket;
	};
}

namespace Sentry
{
	enum DataMessageType
	{
		TYPE_REQUEST = 0X01,        //服务器请求
		TYPE_RESPONSE = 0X02,       //服务器返回
        TYPE_CLIENT_REQUEST = 0X03, //客户端请求
	};
	enum class SocketType
	{
		NoneSocket,
		LocalSocket,
		RemoteSocket
	};
}