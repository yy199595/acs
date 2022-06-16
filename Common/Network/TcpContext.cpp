//
// Created by zmhy0073 on 2022/1/15.
//
#include"TcpContext.h"
#include<iostream>
#include<stdlib.h>
#include"Util/TimeHelper.h"
#include"Util/StringHelper.h"
#include"Define/CommonLogDef.h"
namespace Tcp
{
	TcpContext::TcpContext(std::shared_ptr<SocketProxy> socket)
		: mNetworkThread(socket->GetThread())
	{
		this->mSocket = socket;
		this->mLastOperTime = 0;
		this->mRecvBuffer = nullptr;
	}

	void TcpContext::SetBufferCount(int count, int maxCount)
	{
		this->mBufferCount = count;
		this->mBufferMaxCount = maxCount;
		this->mRecvBuffer = new char[count];
	}

	TcpContext::~TcpContext()
	{
		if(this->mRecvBuffer != nullptr)
		{
			delete[]this->mRecvBuffer;
		}
	}

	void TcpContext::Connect()
	{
		unsigned short port =this->mSocket->GetPort();
		const std::string & ip = this->mSocket->GetIp();
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
		auto address = asio::ip::make_address_v4(ip);
		asio::ip::tcp::endpoint endPoint(address, port);
		std::shared_ptr<TcpContext> self = this->shared_from_this();
		tcpSocket.async_connect(endPoint, [this, self]
			(const asio::error_code& code)
		{
			this->OnConnect(code);
		});
		this->mLastOperTime = Helper::Time::GetNowSecTime();
	}

	void TcpContext::ReceiveHead(int length)
	{
		assert(this->mRecvBuffer);
		this->mLastOperTime = Helper::Time::GetNowSecTime();
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
		std::shared_ptr<TcpContext> self = this->shared_from_this();
		tcpSocket.async_read_some(asio::buffer(this->mRecvBuffer, length),
			[this, self](const asio::error_code & code, size_t size)
			{
				//assert(size > 0);
				if(code || size == 0)
				{
					this->OnReceiveHead(code, nullptr, 0);
					return;
				}
                this->OnReceiveHead(code, this->mRecvBuffer, size);
			});
	}

	void TcpContext::ReceiveBody(int length)
	{
		if(length >= this->mBufferMaxCount)
		{
			asio::error_code code = std::make_error_code(std::errc::bad_message);
			this->OnReceiveBody(code, nullptr, 0);
			return;
		}
		char * bufer = this->mRecvBuffer;
		if(length > this->mBufferCount)
		{
			bufer = new char[length];
		}
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
		std::shared_ptr<TcpContext> self = this->shared_from_this();
		tcpSocket.async_read_some(asio::buffer(bufer, length),
			[this, bufer, self](const asio::error_code & code, size_t size)
			{
                this->OnReceiveBody(code, bufer, size);
				if(bufer != this->mRecvBuffer)
				{
					delete [] bufer;
				}
			});
	}

	void TcpContext::Send(std::shared_ptr<ProtoMessage> message)
	{
		std::ostream os(&this->mSendBuffer);
		const int length = message->Serailize(os);
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
		std::shared_ptr<TcpContext> self = this->shared_from_this();
		asio::async_write(tcpSocket, this->mSendBuffer, [this, self, message, length]
			(const asio::error_code& code, size_t size)
		{
			if(length > 0)
			{
				this->Send(message);
				return;
			}
			this->OnSendMessage(code, message);
		});

		this->mLastOperTime = Helper::Time::GetNowSecTime();
	}
}