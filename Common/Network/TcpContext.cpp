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
		: mNetworkThread(socket->GetThread()), mMaxCount(count)
	{
		this->mSocket = socket;
		this->mLastOperTime = 0;
        this->mReadState = ReadType::HEAD;
	}

	TcpContext::~TcpContext()
	{

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

    void TcpContext::ReceiveLine()
    {
        this->mRecvBuffer.clear();
        asio::streambuf streambuf1;
        this->mLastOperTime = Helper::Time::GetNowSecTime();
        AsioTcpSocket &tcpSocket = this->mSocket->GetSocket();
        std::shared_ptr<TcpContext> self = this->shared_from_this();
        asio::async_read_until(tcpSocket, streambuf1, "\r\n",[](){

        });

        asio::async_read(tcpSocket, streambuf1, [](){

        });
        asio::async_read_until(tcpSocket, asio::dynamic_buffer(this->mRecvBuffer), "\r\n",
                               [this, self](const asio::error_code &code, size_t size) {
                                   if (size >= 2)
                                   {
                                       this->mRecvBuffer.pop_back();
                                       this->mRecvBuffer.pop_back();
                                   }
                                   this->OnReceiveLine(code, this->mRecvBuffer);
                               });
    }

	void TcpContext::ReceiveMessage(int length)
	{
        if(length >= this->mMaxCount)
        {
            asio::error_code code = std::make_error_code(std::errc::bad_message);
            this->OnReceiveMessage(code, this->mRecvBuffer);
            return;
        }
        this->mRecvBuffer.clear();
		this->mLastOperTime = Helper::Time::GetNowSecTime();
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
		std::shared_ptr<TcpContext> self = this->shared_from_this();
		asio::async_read(tcpSocket, asio::dynamic_buffer(this->mRecvBuffer, length),
			[this, self](const asio::error_code & code, size_t size)
			{
                this->OnReceiveMessage(code, this->mRecvBuffer);
			});
	}

    int TcpContext::GetLength(const std::string &buffer)
    {
        if(buffer.size() < sizeof(int ))
        {
            return -1;
        }
        int length = 0;
        memcpy(&length, buffer.c_str(), sizeof(int));
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