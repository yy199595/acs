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
		, mSendStream(&this->mSendBuffer), mRecvStream(&mRecvBuffer)
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
				this->OnReceiveLine(code, size);
			});
	}

	void TcpContext::ReceiveSomeMessage()
	{
        if(this->mRecvBuffer.size() > 0)
        {
            asio::error_code code;
			size_t size = this->mRecvBuffer.size();
            this->OnReceiveMessage(code, size);
            return;
        }
        asio::error_code code;
		this->mLastOperTime = Helper::Time::GetNowSecTime();
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();

        if(tcpSocket.available(code) <= 0)
        {
            code = asio::error::eof;
            this->OnReceiveMessage(code, 0);
            return;
        }
		std::shared_ptr<TcpContext> self = this->shared_from_this();
		asio::async_read(tcpSocket, this->mRecvBuffer, asio::transfer_at_least(1),
			[this, self](const asio::error_code& code, size_t size)
			{
				this->OnReceiveMessage(code, size);
			});
	}

	void TcpContext::ReceiveMessage(int length)
	{
		if (length >= this->mMaxCount)
		{
			asio::error_code code = std::make_error_code(std::errc::bad_message);
			this->OnReceiveMessage(code, 0);
			return;
		}
		if (this->mRecvBuffer.size() >= length)
		{
			asio::error_code code;
			this->OnReceiveMessage(code, length);
			return;
		}
		length -= this->mRecvBuffer.size();
		this->mLastOperTime = Helper::Time::GetNowSecTime();
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
		std::shared_ptr<TcpContext> self = this->shared_from_this();
		asio::async_read(tcpSocket, this->mRecvBuffer, asio::transfer_exactly(length),
			[this, self](const asio::error_code& code, size_t size)
			{
				this->OnReceiveMessage(code, size);
			});
	}

    int TcpContext::GetLength()
	{
		if (this->mRecvBuffer.size() < sizeof(int))
		{
			return -1;
		}
		int length = 0;
		char temp[sizeof(int)] = { 0 };
		this->mRecvStream.readsome(temp, sizeof(int));
		memcpy(&length, temp, sizeof(int));
		return length;
	}

	void TcpContext::Send(std::shared_ptr<ProtoMessage> message)
	{
		const int length = message->Serailize(this->mSendStream);
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

namespace Tcp
{
	void TcpContext::ClearRecvStream()
	{
		if(this->mRecvBuffer.size() > 0)
		{
			std::iostream os(&this->mRecvBuffer);
			os.ignore(this->mRecvBuffer.size());
		}
	}

	void TcpContext::ClearSendStream()
	{
		if(this->mRecvBuffer.size() > 0)
		{
			std::iostream os(&this->mRecvBuffer);
			os.ignore(this->mRecvBuffer.size());
		}
	}

	bool TcpContext::ConnectSync()
	{
		asio::error_code code;
		AsioTcpSocket & tcpSocket = this->mSocket->GetSocket();
		auto address = asio::ip::make_address_v4(this->mSocket->GetIp());
		asio::ip::tcp::endpoint endPoint(address, this->mSocket->GetPort());
		tcpSocket.connect(endPoint, code);
		if(code)
		{
			printf("sync connect [%s] failure", this->mSocket->GetAddress().c_str());
			return false;
		}
		return true;
	}

	int TcpContext::RecvSync(int length)
	{
		asio::error_code code;
		if(this->mRecvBuffer.size() >= length)
		{
			return length;
		}
		AsioTcpSocket & tcpSocket = this->mSocket->GetSocket();
		int size = asio::read(tcpSocket, this->mRecvBuffer,
			asio::transfer_exactly(length), code);
		return code ? 0 : size;
	}

	int TcpContext::RecvLineSync()
	{
		asio::error_code code;
		AsioTcpSocket & tcpSocket = this->mSocket->GetSocket();
		return asio::read_until(tcpSocket, this->mRecvBuffer, "\r\n", code);
	}

	int TcpContext::SendSync(std::shared_ptr<ProtoMessage> message)
	{
		int sum = 0;
		asio::error_code code;
		int length = message->Serailize(this->mSendStream);
		AsioTcpSocket & tcpSocket = this->mSocket->GetSocket();
		sum += asio::write(tcpSocket, this->mSendBuffer, code);
		while(length > 0 && sum > 0 && !code)
		{
			try
			{
				sum += asio::write(tcpSocket, this->mSendBuffer);
				length = message->Serailize(this->mSendStream);
			}
			catch (std::system_error & error)
			{
				printf("sync send failure %s\n", error.what());
				return 0;
			}
		}
		return sum;
	}

}