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
        SocketProxy(NetWorkThread & thread, const std::string & name, std::shared_ptr<AsioTcpSocket> socket);
        ~SocketProxy() = default;
	public:
		void Close();
        void RefreshState();
        bool IsOpen() { return this->mIsOpen; }
		AsioTcpSocket & GetSocket() { return *mSocket; }
		NetWorkThread & GetThread() { return this->mNetThread; }
		long long GetSocketId() const { return this->mSocketId; }
		const std::string & GetName() const { return this->mName; }
        const std::string & GetAddress() { return this->mAddress; }
        AsioContext & GetContext() { return this->mNetThread.GetContext(); }
    private:
		long long mSocketId;
        std::string mAddress;
		const std::string mName;
        std::atomic_bool mIsOpen;
        NetWorkThread & mNetThread;
        std::shared_ptr<AsioTcpSocket> mSocket;
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