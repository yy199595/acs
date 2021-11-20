#include"RpcClient.h"
#include"Util/TimeHelper.h"
namespace GameKeeper
{
	RpcClient::RpcClient(SocketProxy * socket)
		: mSocketProxy(socket), mContext(socket->GetContext()),
		mNetWorkThread(socket->GetThread())
	{
		this->mRecvBuffer = 0;
		this->mSocketId = socket->GetSocketId();
	}

	void RpcClient::StartReceive()
	{
		AsioTcpSocket & socket = this->mSocketProxy->GetSocket();
		if (this->mRecvBuffer == nullptr)
		{
			this->mRecvBuffer = new char[TCP_BUFFER_COUNT];			
		}
		this->mLastOperTime = TimeHelper::GetSecTimeStamp();
		unsigned short port = socket.remote_endpoint().port();
		this->mIp = socket.remote_endpoint().address().to_string();
		this->mAddress = this->mIp + ":" + std::to_string(port);
		if (mNetWorkThread.IsCurrentThread())
		{
			this->ReceiveHead();
			return;
		}
		mNetWorkThread.AddTask(&RpcClient::ReceiveHead, this);
	}

	void RpcClient::ReceiveHead()
	{		
		if (!this->mSocketProxy->IsOpen())
		{
			this->CloseSocket(XCode::NetWorkError);
			return;
		}
		AsioTcpSocket & socket = this->mSocketProxy->GetSocket();
		const size_t count = sizeof(unsigned int) + sizeof(char);
		socket.async_read_some(asio::buffer(this->mRecvBuffer, count),
			[this](const asio::error_code &error_code, const std::size_t t)
		{
			this->mLastOperTime = TimeHelper::GetSecTimeStamp();
			if (error_code)
			{
				GKDebugError(error_code.message());
				this->CloseSocket(XCode::NetReceiveFailure);
			}
			else
			{
				unsigned int length = 0;
				char type = this->mRecvBuffer[0];
				memcpy(&length, this->mRecvBuffer + sizeof(char), sizeof(unsigned int));
				if (length >= MAX_DATA_COUNT)
				{
					this->CloseSocket(XCode::NetBigDataShutdown);
					return;
				}

				if (type == RPC_TYPE_REQUEST || type == RPC_TYPE_RESPONSE)
				{
					this->ReceiveBody(type, length);
					return;
				}
				GKDebugFatal("receive err type = " << type);
				this->CloseSocket(XCode::NetReceiveFailure);
			}
		});
	}

	void RpcClient::ReceiveBody(char type, size_t size)
	{
		char * messageBuffer = this->mRecvBuffer;
		if (size > TCP_BUFFER_COUNT)
		{
			messageBuffer = new char[size];
			GKDebugWarning("receive message count = " << size);
		}
		this->mLastOperTime = TimeHelper::GetSecTimeStamp();
		this->mLastOperTime = TimeHelper::GetSecTimeStamp();
		AsioTcpSocket & nSocket = this->mSocketProxy->GetSocket();

		asio::error_code code;
		if (size > nSocket.available(code))
		{
			this->CloseSocket(XCode::NetReceiveFailure);
			GKDebugError("available data less " << size);
			return;
		}
		nSocket.async_read_some(asio::buffer(messageBuffer, size),
			[this, messageBuffer, type](const asio::error_code &error_code,
				const std::size_t size) 
		{
			if (error_code)
			{
				GKDebugError(error_code.message());
				this->CloseSocket(XCode::NetReceiveFailure);
			}
			else if(type == RPC_TYPE_REQUEST)
			{
				if (!this->OnRequest(messageBuffer, size))
				{
					this->CloseSocket(XCode::ParseMessageError);
					GKDebugError(this->GetAddress() << " parse request message error");
				}
			}
			else if (type == RPC_TYPE_RESPONSE)
			{
				if (!this->OnResponse(messageBuffer, size))
				{
					this->CloseSocket(XCode::ParseMessageError);
					GKDebugError(this->GetAddress() << " parse response message error");
				}
			}
		});
	}

	bool RpcClient::AsyncSendMessage(char * buffer, size_t size)
	{
		if (!this->mSocketProxy->IsOpen())
		{
			this->CloseSocket(XCode::NetWorkError);
			return false;
		}
		AsioTcpSocket & nSocket = this->mSocketProxy->GetSocket();
		nSocket.async_send(asio::buffer(buffer, size), [buffer, this]
				(const asio::error_code &error_code, std::size_t size)
		{
			XCode code = XCode::Successful;
			if (error_code)
			{
				code = XCode::NetSendFailure;
				GKDebugError(error_code.message());
				this->CloseSocket(XCode::NetSendFailure);
			}
			delete[]buffer;
			this->mLastOperTime = TimeHelper::GetSecTimeStamp();
		});
		return true;
	}

}