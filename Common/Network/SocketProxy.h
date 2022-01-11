#pragma once
#include<mutex>
#include<Define/CommonLogDef.h>
#include<Thread/TaskThread.h>
#include<Pool/StringPool.h>

namespace GameKeeper
{
	class SocketProxy
	{
	public:
		SocketProxy(NetWorkThread & thread, const std::string & name);
		~SocketProxy() = default;
	public:
		void Close();
		bool IsOpen();	
		AsioTcpSocket & GetSocket() { return this->mSocket; }
		AsioContext & GetContext() { return this->mContext; }
		NetWorkThread & GetThread() { return this->mNetThread; }
		long long GetSocketId() const { return this->mSocketId; }
		const std::string & GetName() const { return this->mName; }
	private:
		std::mutex mLock;
		long long mSocketId;
		AsioTcpSocket mSocket;		
		AsioContext & mContext;
		const std::string mName;
		NetWorkThread & mNetThread;
	};
}

namespace GameKeeper
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