#include"TcpClientSession.h"
#include<Util/TimeHelper.h>
#include<Network/Tcp/TcpClientComponent.h>

namespace GameKeeper
{
    TcpClientSession::TcpClientSession(TcpClientComponent *component)
            :mTcpComponent(component)
    {      		
		this->mLastOperTime = 0;
		this->mSocketProxy = nullptr;
		this->mReceiveMsgBuffer = new char[TCP_BUFFER_COUNT];
    }

    TcpClientSession::~TcpClientSession()
    {
        delete this->mSocketProxy;
        delete[]this->mReceiveMsgBuffer;
    }

	void TcpClientSession::SetSocket(SocketProxy * socketProxy)
    {
        delete this->mSocketProxy;
        this->mSocketProxy = socketProxy;     
        if (this->mSocketProxy->IsOpen())
        {
            this->StartReceive();
			AsioTcpSocket &nScoket = this->mSocketProxy->GetSocket();
            this->mAddress = nScoket.remote_endpoint().address().to_string()
                             + ":" + std::to_string(nScoket.remote_endpoint().port());
        }
    }


	void TcpClientSession::StartClose()
	{		
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		if (nThread.IsCurrentThread())
		{
			this->CloseSocket(XCode::NetActiveShutdown);
			return;
		}
		nThread.AddTask(&TcpClientSession::CloseSocket, this, XCode::NetActiveShutdown);
		
	}

	void TcpClientSession::StartReceive()
	{		
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		if (nThread.IsCurrentThread())
		{
			this->ReceiveMessage();
			return;
		}
		nThread.AddTask(&TcpClientSession::ReceiveMessage, this);
		
	}

	void TcpClientSession::StartSendByString(std::string * message)
	{		
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		if (nThread.IsCurrentThread())
		{
			this->SendByString(message);
			return;
		}
		nThread.AddTask(&TcpClientSession::SendByString, this, message);
		
	}


	void TcpClientSession::ReceiveMessage()
	{
		this->mLastOperTime = TimeHelper::GetSecTimeStamp();
		AsioTcpSocket & socket = this->mSocketProxy->GetSocket();

		socket.async_read_some(asio::buffer(this->mReceiveMsgBuffer, sizeof(unsigned int)),
			[this](const asio::error_code &error_code, const std::size_t t)
		{
			if (error_code)
			{
				GKDebugError(error_code.message());
				this->CloseSocket(XCode::NetReceiveFailure);		
			}
			else
			{
				size_t packageSize = 0;
				memcpy(&packageSize, this->mReceiveMsgBuffer, t);
				if (packageSize >= 1024 * 10) //最大为10k
				{
					this->CloseSocket(XCode::NetBigDataShutdown);
					return;
				}
				this->ReadMessageBody(packageSize);
			}
			this->mLastOperTime = TimeHelper::GetSecTimeStamp();
		});
	}

	void TcpClientSession::CloseSocket(XCode code)
	{		
		if (this->mSocketProxy->IsOpen())
		{
			asio::error_code err;
			this->mSocketProxy->GetSocket().close(err);
		}
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		nThread.AddTask(&TcpClientComponent::OnCloseSession, this->mTcpComponent, this, code);
	}

	void TcpClientSession::ReadMessageBody(const size_t allSize)
	{	
		if (!this->mSocketProxy->IsOpen())
		{
			this->CloseSocket(XCode::NetWorkError);
			return;
		}
		char *nMessageBuffer = this->mReceiveMsgBuffer;
		if (allSize > TCP_BUFFER_COUNT)
		{
			nMessageBuffer = new char[allSize];
		}
		this->mLastOperTime = TimeHelper::GetSecTimeStamp();
		AsioTcpSocket & nSocket = this->mSocketProxy->GetSocket();

		asio::error_code code;
		if (allSize < nSocket.available(code))
		{
			this->CloseSocket(XCode::NetReceiveFailure);
			return;
		}
		nSocket.async_read_some(asio::buffer(nMessageBuffer, allSize),
			[this, nMessageBuffer](const asio::error_code &error_code,
				const std::size_t size) {
			if (error_code)
			{
				GKDebugError(error_code.message());
				this->CloseSocket(XCode::NetReceiveFailure);			
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
			this->mLastOperTime = TimeHelper::GetSecTimeStamp();
		});
	}

	void TcpClientSession::SendByString(std::string * message)
	{		
		this->mLastOperTime = TimeHelper::GetSecTimeStamp();
		AsioTcpSocket & nSocket = this->mSocketProxy->GetSocket();
		if (!this->mSocketProxy->IsOpen())
		{
			this->CloseSocket(XCode::HttpNetWorkError);
			NetWorkThread & nThread = this->mSocketProxy->GetThread();
			nThread.AddTask(&TcpClientComponent::OnSendMessageAfter,
				this->mTcpComponent, this, message, XCode::NetWorkError);
		}
		else
		{
			nSocket.async_send(asio::buffer(message->c_str(), message->size()),
				[message, this](const asio::error_code &error_code, std::size_t size)
			{
				XCode code = XCode::Successful;
				if (error_code)
				{
					code = XCode::NetSendFailure;
					this->CloseSocket(XCode::NetSendFailure);
				}
				this->mLastOperTime = TimeHelper::GetSecTimeStamp();
				NetWorkThread & nThread = this->mSocketProxy->GetThread();
				nThread.AddTask(&TcpClientComponent::OnSendMessageAfter,  this->mTcpComponent, this, message, code);
			});
		}		
	}
}