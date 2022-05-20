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
		: mContext(socket->GetContext()), mNetworkThread(socket->GetThread())
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

	void TcpContext::ReceiveHead()
	{
		assert(this->mRecvBuffer);
		this->mLastOperTime = Helper::Time::GetNowSecTime();
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
		std::shared_ptr<TcpContext> self = this->shared_from_this();
		tcpSocket.async_read_some(asio::buffer(this->mRecvBuffer, sizeof(int)),
			[this, self](const asio::error_code & code, size_t size)
			{
				//assert(size > 0);
				if(code || size == 0)
				{
					printf("code = %s\n", code.message().c_str());
					this->OnRecvMessage(code, nullptr, 0);
					return;
				}
				int lenght = 0;
				memcpy(&lenght, this->mRecvBuffer, sizeof(int));
				if(lenght <= 0)
				{
					this->OnRecvMessage(std::make_error_code(std::errc::no_message), nullptr, 0);
					return;
				}
				this->ReceiveBody(lenght);
			});
	}

	void TcpContext::ReceiveBody(int length)
	{
		if(length >= this->mBufferMaxCount)
		{
			asio::error_code code = std::make_error_code(std::errc::bad_message);
			this->OnRecvMessage(code, nullptr, 0);
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
				if(this->OnRecvMessage(code, bufer, size))
				{
					this->mContext.post(std::bind(&TcpContext::ReceiveHead, this));
				}
				if(bufer != this->mRecvBuffer)
				{
					delete [] bufer;
				}
			});
	}

	void TcpContext::Send(std::shared_ptr<ProtoMessage> message)
	{
		std::ostream os(&this->mSendBuffer);
		this->mLastOperTime = Helper::Time::GetNowSecTime();
		if (message->Serailize(os))
		{
			AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
			std::shared_ptr<TcpContext> self = this->shared_from_this();
			asio::async_write(tcpSocket, this->mSendBuffer, [this, self, message]
				(const asio::error_code& code, size_t size)
			{
				this->OnSendMessage(code, message);
			});
		}
	}
}