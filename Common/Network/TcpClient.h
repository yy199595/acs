//
// Created by zmhy0073 on 2022/1/15.
//

#ifndef GAMEKEEPER_TCPCLIENT_H
#define GAMEKEEPER_TCPCLIENT_H
#include"XCode/XCode.h"
#include"SocketProxy.h"
#include<asio/coroutine.hpp>
namespace Sentry
{
	class TcpClient
	{
	 public:
		TcpClient(std::shared_ptr<SocketProxy> socket, SocketType type);
	 public:
		void StartReceive();
		bool IsOpen() const
		{
			return this->mIsOpen;
		}
	 protected:
		virtual void BeginReceive() = 0;
		virtual void OnConnect(const asio::error_code& err) = 0;
		virtual void OnSendData(const asio::error_code& err, int tag) = 0;
	 protected:
		bool StartConnect(const std::string& address);
		bool SendDataByArray(const char* buffer, size_t size, int tag = 0);
	 private:
		void ConnectByAddress(const std::string& ip, unsigned short port);
	 protected:
		std::string mAddress;
		AsioContext& mContext;
		IAsioThread& mNetworkThread;
		std::shared_ptr<SocketProxy> mSocket;
	 private:
		SocketType mSocketType;
		std::atomic_bool mIsOpen;
		std::atomic_bool mIsConnected; //是否正在连接
	};
}
#endif //GAMEKEEPER_TCPCLIENT_H
