#include"TcpClientSession.h"
#include<Component/IComponent.h>

namespace GameKeeper
{
    TcpClientSession::TcpClientSession(ISocketHandler * handler)
            : SessionBase(handler)
    {      
		this->mReceiveBufferSize = 1024;
		this->mReceiveMsgBuffer = nullptr;
    }

    TcpClientSession::~TcpClientSession()
    {
		if (this->mReceiveMsgBuffer)
		{
			delete[]this->mReceiveMsgBuffer;
		}    
    }

	void TcpClientSession::OnSessionEnable()
    {
        if (this->mReceiveMsgBuffer == nullptr)
        {
            this->mReceiveMsgBuffer = new char[this->mReceiveBufferSize];
        }
        this->StartReceive();
    }

	void TcpClientSession::StartReceive()
	{
		this->mSocket->async_read_some(asio::buffer(this->mReceiveMsgBuffer, sizeof(unsigned int)),
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
				memcpy(&packageSize, this->mReceiveMsgBuffer, t);
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
		char *nMessageBuffer = this->mReceiveMsgBuffer;
		if (allSize > this->mReceiveBufferSize)
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
			if (nMessageBuffer != this->mReceiveMsgBuffer)
			{
				delete[]nMessageBuffer;
			}
		});
	}

}