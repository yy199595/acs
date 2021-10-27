#pragma once
#include<atomic>
#include"NetworkHelper.h"
#include<Method/MethodProxy.h>

namespace Sentry
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

namespace Sentry
{
	class ISocketHandler;

	class MainTaskScheduler;

	class SessionBase
	{
	public:
		SessionBase(ISocketHandler *handler);

		~SessionBase()
		{}

	public:
		void Close();

		AsioTcpSocket &GetSocket() { return *mSocket; }

		AsioContext &GetContext() { return this->mContext; }

	public:
		const std::string &GetName() { return this->mName; }

		const std::string &GetLocalAddress() { return this->mLocalAddress; };

		const std::string &GetRemoteAddress() { return this->mRemoteAddress; };

		const std::string &GetAddress() { return this->mRemoteAddress; }

	public:
		void OnListenDone(const asio::error_code & err);

		bool SendNetMessage(std::string *message);

		bool IsActive() { return this->mIsOpen; }

        virtual SocketType GetSocketType() = 0;

	protected:
		virtual void OnClose();

		virtual void OnSessionEnable() { };

		virtual void OnError(const asio::error_code &err);

		virtual void OnConnect(const asio::error_code &err);

		virtual void OnReceiveMessage(const char *msg, size_t size); //子类调用

		virtual void OnSendByString(std::string *msg, const asio::error_code &err);

		virtual void OnSendByStream(asio::streambuf *msg, const asio::error_code &err);

	protected:

		void InitMember();

		void SendByString(std::string *message);

		void SendByStream(asio::streambuf * message);
	protected:
		std::string mName;
		SharedTcpSocket mSocket;
		ISocketHandler *mHandler;
		MainTaskScheduler &mTaskScheduler;
	private:
		std::string mLocalAddress;
		std::string mRemoteAddress;
	private:
		AsioContext &mContext;
		std::atomic_bool mIsOpen;
	};
}