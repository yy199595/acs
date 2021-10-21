#include"TcpClientSession.h"
#include<Component/IComponent.h>
#include<TcpNetwork/SocketEvent.h>

namespace Sentry
{
    TcpClientSession::TcpClientSession(ISocketHandler * handler)
            : SessionBase(handler)
    {      
		this->mRecvBufferSize = 1024;
		this->mRecvMsgBuffer = nullptr;
    }

    TcpClientSession::~TcpClientSession()
    {
		if (this->mRecvMsgBuffer != nullptr)
		{
			delete[]this->mRecvMsgBuffer;
		}    
    }

	void TcpClientSession::OnStartReceive()
	{
		if (this->mRecvMsgBuffer == nullptr)
		{
			this->mRecvMsgBuffer = new char[this->mRecvBufferSize];
		}
		this->mSocket->async_read_some(asio::buffer(this->mRecvMsgBuffer, sizeof(unsigned int)),
			[this](const asio::error_code &error_code, const std::size_t t)
		{
			if (error_code)
			{
				this->OnClose();
				SayNoDebugError(error_code.message());			
			}
			else
			{
				size_t packageSize = 0;
				memcpy(&packageSize, this->mRecvMsgBuffer, t);
				if (packageSize >= 1024 * 10) //最大为10k
				{
					this->OnClose();
					return;
				}
				this->ReadMessageBody(packageSize);
			}
		});
	}

	void TcpClientSession::ReadMessageBody(const size_t allSize)
	{
		char *nMessageBuffer = this->mRecvMsgBuffer;
		if (allSize > this->mRecvBufferSize)
		{
			nMessageBuffer = new char[allSize];
		}

		this->mSocket->async_read_some(asio::buffer(nMessageBuffer, allSize),
			[this, nMessageBuffer](const asio::error_code &error_code,
				const std::size_t messageSize) {
			if (error_code)
			{
				this->OnClose();
				SayNoDebugError(error_code.message());
			}
			else
			{
				this->OnReceiveMessage(nMessageBuffer, messageSize);				
			}	
			if (nMessageBuffer != this->mRecvMsgBuffer)
			{
				delete[]nMessageBuffer;
			}
		});
	}
}