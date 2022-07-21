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
		SocketProxy(IAsioThread& thread, const std::string & ip, unsigned short port);
		~SocketProxy();
	 public:
        void Init();
        void Close();
		unsigned short GetPort() { return this->mPort;}
		inline std::string & GetIp() { return this->mIp;}
		inline bool IsRemote() const { return this->mIsRemote;}
		inline AsioTcpSocket& GetSocket() { return *mSocket;}
		inline IAsioThread& GetThread() { return this->mNetThread; }
		inline const std::string& GetAddress() { return this->mAddress; }
	 private:
		bool mIsRemote;
		std::string mIp;
		std::mutex mLock;
		unsigned short mPort;
		std::string mAddress;
		IAsioThread& mNetThread;
		AsioTcpSocket * mSocket;
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