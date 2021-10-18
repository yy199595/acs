#pragma once

#include<Method/MethodProxy.h>
#include<Define/CommonTypeDef.h>
#include<atomic>
namespace Sentry
{
	enum DataMessageType
	{
		TYPE_REQUEST = 1,
		TYPE_RESPONSE = 2
	};
}

namespace Sentry
{
	class ISocketHandler;
	class MainTaskScheduler;
	class SessionBase
	{
	public:
		SessionBase(ISocketHandler * handler);
		~SessionBase() { }
	public:
		void Close();
		AsioTcpSocket & GetSocket() { return *mSocket; }
        AsioContext & GetContext() { return this->mContext; }
	public:
        const std::string & GetName() { return this->mName; }
        const std::string & GetLocalAddress() { return this->mLocalAddress;};
        const std::string & GetRemoteAddress() { return this->mRemoteAddress;};
		void StartConnect(std::string name, std::string host, unsigned short port);
        const std::string & GetAddress() { return this->mIsConnected ? this->mLocalAddress : this->mRemoteAddress;}
	public:
        void OnListenDone();
        bool SendNetMessage(SharedMessage message);
        bool IsActive() { return this->mIsOpen; }
        bool IsConnected() { return this->mIsConnected; }
	protected:
		void OnClose();
		void OnError(const asio::error_code &err);
		void OnConnect(const asio::error_code & err);
		void OnReceiveMessage(const char * msg, size_t size); //子类调用
	protected:
		virtual void OnStartReceive() = 0;
	private:
		void OnSendMessage(SharedMessage message);
        void ConnectHandler(const asio::error_code & err);
        void OnStartConnect(std::string name, std::string ip, unsigned short port);
    protected:
        std::string mName;
        SharedTcpSocket mSocket;
    private:
        std::string mLocalAddress;
        std::string mRemoteAddress;
    private:
		bool mIsConnected;
        int mConnectCount;
        AsioContext & mContext;
		ISocketHandler * mHandler;
		MainTaskScheduler & mTaskScheduler;
        std::atomic_bool mIsOpen;
	};
}