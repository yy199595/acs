//
// Created by zmhy0073 on 2022/1/15.
//
#include"TcpContext.h"
#include<iostream>
#include<stdlib.h>
#include"Util/StringHelper.h"
#include"Define/CommonLogDef.h"
namespace Sentry
{
	TcpContext::TcpContext(std::shared_ptr<SocketProxy> socket, SocketType type)
		: mContext(socket->GetContext()), mNetworkThread(socket->GetThread())
	{
		this->mSocket = socket;
		this->mSocketType = type;
	}

	bool TcpContext::StartConnect(const std::string& address)
	{
		if (this->mIsConnected)
		{
			return false;
		}
		std::string ip;
		unsigned short port = 0;
		this->mIsConnected = true;
		assert(Helper::String::ParseIpAddress(address, ip, port));

#ifdef ONLY_MAIN_THREAD
		this->ConnectByAddress(ip, port);
#else
		this->mNetworkThread.Invoke(&TcpContext::ConnectByAddress, this, ip, port);
#endif
		return true;
	}

	void TcpContext::StartReceive()
	{
		this->mIsOpen = true;
#ifdef ONLY_MAIN_THREAD
		this->BeginReceive();
#else
		this->mNetworkThread.Invoke(&TcpContext::BeginReceive, this);
#endif
	}

	void TcpContext::ConnectByAddress(const std::string& ip, unsigned short port)
	{
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
		auto address = asio::ip::make_address_v4(ip);
		asio::ip::tcp::endpoint endPoint(address, port);
		tcpSocket.async_connect(endPoint, [this]
			(const asio::error_code& code)
		{
			if (code)
			{
				this->mIsOpen = false;
			}
			this->mIsConnected = false;
			this->OnConnect(code);
		});
	}

	bool TcpContext::SendDataByArray(const char* buffer, size_t size, int tag)
	{
		if (!this->mIsOpen)
		{
			return false;
		}
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
		tcpSocket.async_send(asio::buffer(buffer, size), [this, tag](
			const asio::error_code& error, const size_t size)
		{
			if (error)
			{
				this->mIsOpen = false;
			}
			this->OnSendData(error, tag);
		});
		return true;
	}
}