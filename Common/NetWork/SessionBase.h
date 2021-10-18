#pragma once

#include<Method/MethodProxy.h>
#include<Define/CommonTypeDef.h>

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
		void SetSocket(SharedTcpSocket socket);
		AsioTcpSocket & GetSocket() { return *mSocket; }
		ISocketHandler & GetHandler() { return *mHandler; }
		AsioContext & GetContext() { return this->mContext; }
	public:
		void StartReceive();
		
		virtual const std::string & GetAddress() = 0;
		void StartConnect(std::string name, std::string ip, unsigned short port);
	public:
		bool IsActive();
		bool SendNetMessage(SharedMessage message);
		bool IsConnected() { return this->mIsConnected; }
	protected:
		void OnClose();
		void OnError(const asio::error_code &err);
		void OnConnect(const asio::error_code & err);
		void OnReceiveMessage(const char * msg, size_t size);
	protected:
		virtual void OnStartReceive() = 0;
		virtual void OnStartConnect(std::string name, std::string ip, unsigned short port) = 0;
	private:
		void OnSendMessage(SharedMessage message);
	protected:
		SharedTcpSocket mSocket;
	private:
		bool mIsOpen;
		bool mIsConnected;
		AsioContext & mContext;		
		ISocketHandler * mHandler;
		MainTaskScheduler & mTaskScheduler;
	};
}