#include"RpcClientContext.h"
#include"App/App.h"
#include"Util/TimeHelper.h"
#include<iostream>

namespace Sentry
{
    NetworkData::NetworkData(char type, std::shared_ptr<Message> message)
    {
        this->mType = type;
        this->mMessage = message;
    }

    size_t NetworkData::GetByteSize()
    {
        return this->mMessage->ByteSize() + sizeof(char) + sizeof(int);
    }

    bool NetworkData::WriteToBuffer(std::string & streamBuffer)
    {
        streamBuffer.clear();
        streamBuffer += this->mType;
        int body = this->mMessage->ByteSize();
        streamBuffer.append((char *) &body, sizeof(int));
        return this->mMessage->AppendToString(&streamBuffer);
    }
}

namespace Sentry
{
	RpcClientContext::RpcClientContext(std::shared_ptr<SocketProxy> socket, SocketType type)
		: mType(type), mSocketProxy(socket),
		  mContext(socket->GetContext()),
		  mNetWorkThread(socket->GetThread())
	{
		this->mIsConnect = false;
		this->mIsCanSendData = true;
		this->mLastOperTime = Helper::Time::GetNowSecTime();
	}

	void RpcClientContext::Clear()
	{
		this->mLastOperTime = 0;
		this->mSocketProxy->Close();
	}

	void RpcClientContext::StartReceive()
	{
		this->mLastOperTime = Helper::Time::GetNowSecTime();
		if (mNetWorkThread.IsCurrentThread())
		{
			this->ReceiveHead();
			return;
		}
		mNetWorkThread.Invoke(&RpcClientContext::ReceiveHead, this);
	}

	void RpcClientContext::ReceiveHead()
	{
		assert(this->mNetWorkThread.IsCurrentThread());
		const size_t count = sizeof(int) + sizeof(char);
		AsioTcpSocket& socket = this->mSocketProxy->GetSocket();
		std::shared_ptr<RpcClientContext> self = this->shared_from_this();
		auto cb = [self, this](const asio::error_code& code, const std::size_t t)
		{
			self->mLastOperTime = Helper::Time::GetNowSecTime();
			if (code)
			{
				this->OnClientError(XCode::NetWorkError);
				std::cerr << FormatFileLine(__FILE__, __LINE__) << code.message() << std::endl;
				return;
			}

			int length = 0;
			char type = self->mReceiveBuffer[0];
			memcpy(&length, self->mReceiveBuffer + sizeof(char), sizeof(int));
			this->ReceiveBody(type, length);
		};
		socket.async_read_some(asio::buffer(this->mReceiveBuffer, count), std::move(cb));
	}

	void RpcClientContext::ReceiveBody(char type, size_t size)
	{
		char* messageBuffer = this->mReceiveBuffer;
		if (size > TCP_BUFFER_COUNT)
		{
			if (size > MAX_DATA_COUNT)
			{
				this->mSocketProxy->Close();
				this->OnClientError(XCode::NetBigDataShutdown);
				return;
			}
			messageBuffer = new char[size];
		}
		this->mLastOperTime = Helper::Time::GetNowSecTime();
		AsioTcpSocket& nSocket = this->mSocketProxy->GetSocket();
		std::shared_ptr<RpcClientContext> self = this->shared_from_this();

		auto cb = [self, this, messageBuffer, type]
			(const asio::error_code& error_code, const size_t size)
		{
			if (error_code)
			{
				this->mSocketProxy->Close();
				this->OnClientError(XCode::NetWorkError);
				std::cerr << FormatFileLine(__FILE__, __LINE__) << error_code.message() << std::endl;
				return;
			}
			if(!this->OnReceiveMessage(type, messageBuffer, size))
			{
				this->mSocketProxy->Close();
				this->OnClientError(XCode::UnKnowPacket);
			}
			else
			{
				mContext.post(std::bind(&RpcClientContext::ReceiveHead, this));
			}
			if (messageBuffer != this->mReceiveBuffer)
			{
				delete[] messageBuffer;
			}
		};
		nSocket.async_read_some(asio::buffer(messageBuffer, size), std::move(cb));
	}

	bool RpcClientContext::StartConnect()
	{
		if (this->IsCanConnection())
		{
			this->mIsConnect = true;
			LOG_CHECK_RET_FALSE(this->mSocketProxy);
#ifdef ONLY_MAIN_THREAD
			this->ConnectHandler();
#else
			IAsioThread &nThread = this->mSocketProxy->GetThread();
			nThread.Invoke(&RpcClientContext::ConnectHandler, this);
#endif
			return true;
		}
		return false;
	}

	void RpcClientContext::ConnectHandler()
	{
		assert(this->mNetWorkThread.IsCurrentThread());
		AsioTcpSocket& nSocket = this->mSocketProxy->GetSocket();
		std::shared_ptr<RpcClientContext> self = this->shared_from_this();
		address_v4 address = asio::ip::make_address_v4(this->mSocketProxy->GetIp());
		asio::ip::tcp::endpoint endPoint(address, this->mSocketProxy->GetPort());

		LOG_DEBUG("start connect " << this->mSocketProxy->GetAddress());
		nSocket.async_connect(endPoint, [self, this](const asio::error_code& err)
		{
			XCode code = XCode::Successful;
			if (err)
			{
				code = XCode::NetConnectFailure;
				std::cerr << FormatFileLine(__FILE__, __LINE__) << err.message() << std::endl;
			}
			this->OnConnect(code);
			this->mIsConnect = false;
		});
	}

	bool RpcClientContext::IsCanConnection()
	{
		return this->GetSocketType() == SocketType::LocalSocket
			   && !this->mSocketProxy->IsOpen() && !this->mIsConnect;
	}

	void RpcClientContext::SendData(std::shared_ptr<NetworkData> message)
	{
		assert(this->mNetWorkThread.IsCurrentThread());
		if (!this->mIsCanSendData)
		{
			this->mWaitSendQueue.emplace(message);
			return;
		}
		this->mIsCanSendData = false;
		message->WriteToBuffer(this->mSendBuffer);
		std::shared_ptr<RpcClientContext> self = this->shared_from_this();
		AsioTcpSocket& tcpSocket = this->mSocketProxy->GetSocket();

		size_t size = this->mSendBuffer.size();
		tcpSocket.async_send(asio::buffer(this->mSendBuffer), [this, self, message]
			(const asio::error_code& code, size_t size)
		{
			if (code)
			{
				this->mSocketProxy->Close();
				this->OnClientError(XCode::NetWorkError);
				this->OnSendData(XCode::NetWorkError, message);
				while (!this->mWaitSendQueue.empty())
				{
					this->OnSendData(XCode::NetWorkError, this->mWaitSendQueue.front());
					this->mWaitSendQueue.pop();
				}
				std::cerr << FormatFileLine(__FILE__, __LINE__) << code.message() << std::endl;
			}
			else
			{
				this->mIsCanSendData = true;
				this->OnSendData(XCode::Successful, message);
				if (!this->mWaitSendQueue.empty())
				{
					this->SendData(this->mWaitSendQueue.front());
					this->mWaitSendQueue.pop();
				}
				this->mLastOperTime = Helper::Time::GetNowSecTime();
			}
		});
	}
}