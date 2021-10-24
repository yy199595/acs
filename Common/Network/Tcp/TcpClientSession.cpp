#include"TcpClientSession.h"
#include<Component/IComponent.h>

namespace Sentry
{
    TcpClientSession::TcpClientSession(ISocketHandler * handler)
            : SessionBase(handler)
    {      
		this->mReceiveBufferSize = 1024;
		this->mReceiveMsgBuffer = nullptr;
    }

    TcpClientSession::~TcpClientSession()
    {
		if (this->mReceiveMsgBuffer != nullptr)
		{
			delete[]this->mReceiveMsgBuffer;
		}    
    }

	bool TcpClientSession::StartConnect(const std::string & name, const std::string & ip, unsigned short port)
	{
		if (!NetworkHelper::IsIp(ip))
		{
			SayNoDebugError(ip << " is not ip");
			return false;
		}
		if (this->IsActive())
		{
			return true;
		}
		this->mName = name;
		this->mHandler->GetNetThread()->AddTask(&TcpClientSession::ConnectHandler, this, ip, port);
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
	void TcpClientSession::ConnectCallback(const asio::error_code & err)
	{
		this->OnConnect(err);
	}

	void TcpClientSession::ConnectHandler(const std::string & ip, unsigned short port)
	{
		auto address = asio::ip::make_address_v4(ip);
		asio::ip::tcp::endpoint endPoint(address, port);
		SayNoDebugLog(this->mName << " start connect " << ip << port);
		this->mSocket->async_connect(endPoint, std::bind(&TcpClientSession::ConnectCallback, this, args1));
	}

}