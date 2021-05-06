#include"NetWorkClient.h"
#include<iostream>
#include<functional>

NetWorkClient::NetWorkClient(asio::io_context & io, const std::string ip, const unsigned short port)
	: mAsioContext(io), mIp(ip), mPort(port)
{
	this->mIsConnect = false;
	this->mAddress = ip + ":" + std::to_string(port);
	this->mTcpSocket = std::make_shared<asio::ip::tcp::socket>(io);
}

void NetWorkClient::StartConnect()
{
	if (this->mTcpSocket != nullptr)
	{
		asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(this->mIp), this->mPort);
		this->mTcpSocket->async_connect(endpoint, [this](const asio::error_code & code)
		{
			if (code)
			{
				this->mIsConnect = false;
				std::cout << code.message() << std::endl;
				return;
			}
			std::cout << "connect " << this->mAddress << " successful" <<std::endl;
			this->mIsConnect = true;
			this->StartRecvMsg();
		});
	}
}

void NetWorkClient::StartRecvMsg()
{
	if (this->mTcpSocket != nullptr && this->mIsConnect)
	{
		this->mTcpSocket->async_read_some(asio::buffer(this->mRecvMessageBuffer, sizeof(unsigned int)),
			[this](const asio::error_code & error_code, const std::size_t t)
		{
			if (error_code)
			{
				this->CloseSocket();
				std::cout << error_code.message() << std::endl;
				return;
			}
			size_t messageCount = 0;
			memcpy(&messageCount, this->mRecvMessageBuffer, sizeof(unsigned int));
			this->ReadMessageBody(messageCount);
		});
	}
}

void NetWorkClient::SendPackage(const std::string & message)
{
	if (this->mTcpSocket != nullptr && this->mIsConnect)
	{
		const char * msg = message.c_str();
		const size_t size = message.size();
		unsigned int t = static_cast<unsigned int>(size);
		memcpy(mSendMessageBuffer, &t, sizeof(unsigned int));
		memcpy(mSendMessageBuffer + sizeof(unsigned int), msg, size);

		this->mTcpSocket->async_send(asio::buffer(mSendMessageBuffer, size+sizeof(unsigned int)),
			[this](const asio::error_code & error_code, std::size_t size)
		{
			if (error_code)
			{
				this->CloseSocket();
				std::cout << error_code.message() << std::endl;
				return;
			}
		
		});
	}
}

bool NetWorkClient::TryGetNetMessage(std::string & message)
{
	if (!this->mRecvMessageQueue.empty())
	{
		message = this->mRecvMessageQueue.front();
		this->mRecvMessageQueue.pop();
		return true;
	}
	return false;
}

void NetWorkClient::ReadMessageBody(size_t size)
{
	if (this->mTcpSocket != nullptr && this->mIsConnect)
	{
		this->mTcpSocket->async_read_some(asio::buffer(this->mRecvMessageBuffer, size),
			[this](const asio::error_code & error_code, const std::size_t t)
		{
			if (error_code)
			{
				this->CloseSocket();
				std::cout << error_code.message() << std::endl;
				return;
			}
			this->mRecvMessageQueue.push(std::string(mRecvMessageBuffer, t));
			this->mAsioContext.post(std::bind(&NetWorkClient::StartRecvMsg, this));
		});
	}
}

void NetWorkClient::CloseSocket()
{
	if (this->mIsConnect)
	{
		asio::error_code code;
		this->mIsConnect = false;
		this->mTcpSocket->shutdown(asio::socket_base::shutdown_both);
		this->mTcpSocket->close(code);
	}
}

std::string NetWorkClient::GetPacketBody(size_t size)
{
	return std::string(this->mRecvMessageBuffer, size);
}
