#pragma once
#include<mutex>
#include<Define/CommonDef.h>
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
	extern StringPool GStringPool;

}

namespace GameKeeper
{
	enum DataMessageType
	{
		TYPE_REQUEST = 1,
		TYPE_RESPONSE = 2
	};
	enum SocketType
	{
		NoneSocket,
		LocalSocket,
		RemoteSocket
	};
}