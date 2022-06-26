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
	TcpContext::TcpContext(std::shared_ptr<SocketProxy> socket, size_t count)
		: mNetworkThread(socket->GetThread()), mMaxCount(count), mRecvBuffer(count)
	{
		this->mSocket = socket;
		this->mLastOperTime = 0;
		this->mConnectCount = 0;
		this->mReadState = ReadType::HEAD;
	}

	TcpContext::~TcpContext()
	{

	}

	void TcpContext::Connect()
	{
		this->mConnectCount++;
		assert(this->mSocket->IsRemote());
		unsigned short port = this->mSocket->GetPort();
		const std::string& ip = this->mSocket->GetIp();
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
		auto address = asio::ip::make_address_v4(ip);
		asio::ip::tcp::endpoint endPoint(address, port);
		std::shared_ptr<TcpContext> self = this->shared_from_this();

		printf("start connect %s\n" , this->mSocket->GetAddress().c_str());

		asio::error_code code;
		tcpSocket.close(code);
		tcpSocket.async_connect(endPoint, [this, self]
			(const asio::error_code& code)
		{
			if(!code)
			{
				this->mConnectCount = 0;
				if(this->mRecvBuffer.size() > 0)
				{
					std::iostream os(&this->mRecvBuffer);
					os.ignore(this->mRecvBuffer.size());
				}
			}
			this->OnConnect(code, this->mConnectCount);
		});
		this->mLastOperTime = Helper::Time::GetNowSecTime();
	}

    void TcpContext::ReceiveLine()
	{
		this->mLastOperTime = Helper::Time::GetNowSecTime();
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
		std::shared_ptr<TcpContext> self = this->shared_from_this();
		asio::async_read_until(tcpSocket, this->mRecvBuffer, "\r\n",
			[this, self](const asio::error_code& code, size_t size)
			{
				this->OnReceiveLine(code, this->mRecvBuffer);
			});
	}

	void TcpContext::ReceiveSomeMessage()
	{
        if(this->mRecvBuffer.size() > 0)
        {
            asio::error_code code;
            this->OnReceiveMessage(code, this->mRecvBuffer);
            return;
        }
        asio::error_code code;
		this->mLastOperTime = Helper::Time::GetNowSecTime();
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();

        if(tcpSocket.available(code) <= 0)
        {
            code = asio::error::eof;
            this->OnReceiveMessage(code, this->mRecvBuffer);
            return;
        }
		std::shared_ptr<TcpContext> self = this->shared_from_this();
		asio::async_read(tcpSocket, this->mRecvBuffer, asio::transfer_at_least(1),
			[this, self](const asio::error_code& code, size_t size)
			{
				this->OnReceiveMessage(code, this->mRecvBuffer);
			});
	}

	void TcpContext::ReceiveMessage(int length)
	{
		if (length >= this->mMaxCount)
		{
			asio::error_code code = std::make_error_code(std::errc::bad_message);
			this->OnReceiveMessage(code, this->mRecvBuffer);
			return;
		}
		if (this->mRecvBuffer.size() >= length)
		{
			asio::error_code code;
			this->OnReceiveMessage(code, this->mRecvBuffer);
			return;
		}
		length -= this->mRecvBuffer.size();
		this->mLastOperTime = Helper::Time::GetNowSecTime();
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
		std::shared_ptr<TcpContext> self = this->shared_from_this();
		asio::async_read(tcpSocket, this->mRecvBuffer, asio::transfer_exactly(length),
			[this, self](const asio::error_code& code, size_t size)
			{
				this->OnReceiveMessage(code, this->mRecvBuffer);
			});
	}

    int TcpContext::GetLength(asio::streambuf &buffer)
	{
		if (buffer.size() < sizeof(int))
		{
			return -1;
		}
		int length = 0;
		char temp[sizeof(int)] = { 0 };
		std::iostream os(&buffer);
		os.readsome(temp, sizeof(int));
		memcpy(&length, temp, sizeof(int));
		return length;
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
			if(code && this->mSendBuffer.size() > 0)
			{
				std::iostream os(&this->mSendBuffer);
				os.ignore(this->mSendBuffer.size());
			}
			if(length > 0 && !code)
			{
				this->Send(message);
				return;
			}

			this->OnSendMessage(code, message);
		});

		this->mLastOperTime = Helper::Time::GetNowSecTime();
	}
}