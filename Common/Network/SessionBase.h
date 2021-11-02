#pragma once
#include<atomic>
#include"NetworkHelper.h"
#include<Method/MethodProxy.h>

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

namespace GameKeeper
{
	class ISocketHandler;

	class MainTaskScheduler;

	class SessionBase
	{
	public:
		explicit SessionBase(ISocketHandler *handler);

		virtual ~SessionBase();

	public:
		void Close();

		AsioTcpSocket &GetSocket() { return *mSocket; }

		AsioContext &GetContext() { return this->mContext; }

	public:

		const std::string &GetAddress() { return this->mAddress; }

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

	protected:

		void SendByString(std::string *message);
			
    private:
        void InitMember();
	protected:
        std::string mAddress;
		SharedTcpSocket mSocket;
		ISocketHandler &mHandler;
        std::atomic_bool mIsOpen;
		MainTaskScheduler &mTaskScheduler;
	private:
		AsioContext &mContext;

	};
}