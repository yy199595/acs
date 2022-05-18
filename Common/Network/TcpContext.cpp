//
// Created by zmhy0073 on 2022/1/15.
//
#include"TcpContext.h"
#include<iostream>
#include<stdlib.h>
#include"Util/StringHelper.h"
#include"Define/CommonLogDef.h"
namespace Tcp
{
	TcpContext::TcpContext(std::shared_ptr<SocketProxy> socket)
		: mContext(socket->GetContext()), mNetworkThread(socket->GetThread())
	{
		this->mSocket = socket;
		this->mRecvMaxSize = this->GetRecvSize();
		this->mRecvBuffer = new char[this->mRecvMaxSize];
	}

	TcpContext::~TcpContext()
	{
		delete []this->mRecvBuffer;
	}

	void TcpContext::SendProtoMessage(std::shared_ptr<ProtoMessage> message)
	{
#ifdef ONLY_MAIN_THREAD
		this->Send(message);
#else
		this->mNetworkThread.Invoke(&TcpContext::Send, this, message);
#endif
	}

	void TcpContext::StartConnect()
	{
#ifdef ONLY_MAIN_THREAD
		this->Connect(ip, port);
#else
		this->mNetworkThread.Invoke(&TcpContext::Connect, this);
#endif
	}

	void TcpContext::StartReceive()
	{
#ifdef ONLY_MAIN_THREAD
		this->ReceiveHead();
#else
		this->mNetworkThread.Invoke(&TcpContext::ReceiveHead, this);
#endif
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
	}

	void TcpContext::ReceiveHead()
	{
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
		std::shared_ptr<TcpContext> self = this->shared_from_this();
		tcpSocket.async_read_some(asio::buffer(this->mRecvBuffer, sizeof(int)),
			[this, self](const asio::error_code & code, size_t size)
			{
				if(!code)
				{
					this->OnError(code);
					return;
				}
				int lenght = 0;
				memcpy(&lenght, this->mRecvBuffer, sizeof(int));
				this->ReceiveBody(lenght);
			});
	}

	void TcpContext::ReceiveBody(int size)
	{
		if(size >= TCP_RECEIVE_MAX_SIZE)
		{
			this->OnError(std::make_error_code(std::errc::bad_message));
			return;
		}
		char * bufer = this->mRecvBuffer;
		if(size > this->mRecvMaxSize)
		{
			bufer = new char[size];
		}
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
		std::shared_ptr<TcpContext> self = this->shared_from_this();
		tcpSocket.async_read_some(asio::buffer(bufer, size),
			[this, bufer, self](const asio::error_code & code, size_t size)
			{
				if(!code)
				{
					this->OnError(code);
					return;
				}
				if(this->OnRecvMessage(bufer, size))
				{
					this->mContext.post(std::bind(&TcpContext::ReceiveHead, this));
				}
			});
	}

	void TcpContext::Send(std::shared_ptr<ProtoMessage> message)
	{
		std::ostream os(&this->mSendBuffer);
		if (message->Serailize(os))
		{
			AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
			std::shared_ptr<TcpContext> self = this->shared_from_this();
			asio::async_write(tcpSocket, this->mSendBuffer, [this, self]
				(const asio::error_code& code, size_t size)
			{
				if(code)
				{
					this->OnError(code);
				}
			});
		}
	}
}