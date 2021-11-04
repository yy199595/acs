#include"TcpClientSession.h"
#include<Network/Tcp/TcpClientComponent.h>

namespace GameKeeper
{
    TcpClientSession::TcpClientSession(TcpClientComponent *component)
            :mTcpComponent(component)
    {      		
		this->mSocketProxy = nullptr;
		this->mReceiveMsgBuffer = new char[TCP_BUFFER_COUNT];
    }

    TcpClientSession::~TcpClientSession()
    {
		delete this->mSocketProxy;
		if (this->mReceiveMsgBuffer)
		{
			delete[]this->mReceiveMsgBuffer;
		}
    }

	bool TcpClientSession::IsOpen()
	{
		if (this->mSocketProxy == nullptr)
		{
			return false;
		}
		std::lock_guard<std::mutex> lock(this->mLock);
		return this->mSocketProxy->GetSocket().is_open();
	}

	void TcpClientSession::SetSocket(SocketProxy * socketProxy)
	{
		if (this->mSocketProxy != nullptr)
		{
			delete this->mSocketProxy;
		}
		this->mSocketProxy = socketProxy;
		AsioTcpSocket & nScoket = this->mSocketProxy->GetSocket();
		if (nScoket.is_open())
		{
			this->StartReceive();
			this->mAddress = nScoket.remote_endpoint().address().to_string()
				+ ":" + std::to_string(nScoket.remote_endpoint().port());
		}
	}


	void TcpClientSession::StartClose()
	{		
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		if (!nThread.IsCurrentThread())
		{
			nThread.AddTask(&TcpClientSession::CloseSocket, this);
			return;
		}
		this->CloseSocket();
	}

	void TcpClientSession::StartReceive()
	{		
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		if (!nThread.IsCurrentThread())
		{
			nThread.AddTask(&TcpClientSession::ReceiveMessage, this);
			return;
		}
		this->ReceiveMessage();
	}

	void TcpClientSession::StartSendByString(std::string * message)
	{		
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		if (!nThread.IsCurrentThread())
		{
			nThread.AddTask(&TcpClientSession::SendByString, this, message);
			return;
		}
		this->SendByString(message);
	}


	void TcpClientSession::ReceiveMessage()
	{
		AsioTcpSocket & socket = this->mSocketProxy->GetSocket();
		socket.async_read_some(asio::buffer(this->mReceiveMsgBuffer, sizeof(unsigned int)),
			[this](const asio::error_code &error_code, const std::size_t t)
		{
			if (error_code)
			{
				this->CloseSocket();
				GKDebugError(error_code.message());
			}
			else
			{
				size_t packageSize = 0;
				memcpy(&packageSize, this->mReceiveMsgBuffer, t);
				if (packageSize >= 1024 * 10) //最大为10k
				{
					this->CloseSocket();
					return;
				}
				this->ReadMessageBody(packageSize);
			}
		});
	}

	void TcpClientSession::CloseSocket()
	{		
		AsioTcpSocket & nSocket = this->mSocketProxy->GetSocket();
		if (nSocket.is_open())
		{
			asio::error_code code;
			nSocket.close(code);
		}
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		nThread.AddTask(&TcpClientComponent::OnCloseSession, this->mTcpComponent, this);
	}

	void TcpClientSession::ReadMessageBody(const size_t allSize)
	{
		AsioTcpSocket & nSocket = this->mSocketProxy->GetSocket();
		if (!nSocket.is_open())
		{
			this->CloseSocket();
			return;
		}
		char *nMessageBuffer = this->mReceiveMsgBuffer;
		if (allSize > this->mReceiveBufferSize)
		{
			nMessageBuffer = new char[allSize];
		}
		AsioTcpSocket & nSocket = this->mSocketProxy->GetSocket();
		nSocket.async_read_some(asio::buffer(nMessageBuffer, allSize),
			[this, nMessageBuffer](const asio::error_code &error_code,
				const std::size_t size) {
			if (error_code)
			{
				this->CloseSocket();
				GKDebugError(error_code.message());
			}
			else
			{
				NetWorkThread & thread = this->mSocketProxy->GetThread();		
				std::string * data = GStringPool.New(nMessageBuffer, size);
				thread.AddTask(&TcpClientComponent::OnReceiveMessage, this->mTcpComponent, this, data);					
			}
			if (nMessageBuffer != this->mReceiveMsgBuffer)
			{
				delete[]nMessageBuffer;
			}
		});
	}

	void TcpClientSession::SendByString(std::string * message)
	{		
		
		AsioTcpSocket & nSocket = this->mSocketProxy->GetSocket();
		if (!nSocket.is_open())
		{
			this->CloseSocket();
			NetWorkThread & nThread = this->mSocketProxy->GetThread();
			nThread.AddTask(&TcpClientComponent::OnSendMessageAfter,
				this->mTcpComponent, this, message, false);
			return;
		}
		nSocket.async_send(asio::buffer(message->c_str(), message->size()),
			[message, this](const asio::error_code &error_code, std::size_t size)
		{
			bool isSuccess = error_code.operator bool;
			NetWorkThread & nThread = this->mSocketProxy->GetThread();
			nThread.AddTask(&TcpClientComponent::OnSendMessageAfter,
				this->mTcpComponent, this, message, isSuccess);
		});
	}

}