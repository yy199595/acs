#include "TcpClientSession.h"
#include<Core/Applocation.h>
#include<Manager/SessionManager.h>
namespace SoEasy
{
	TcpClientSession::TcpClientSession(SessionManager * manager, SharedTcpSocket socket)
		:mAsioContext(manager->GetAsioContext()), mCurrentSatte(None)
	{
		this->mIsContent = false;
		this->mBinTcpSocket = socket;
		
		if (this->mBinTcpSocket != nullptr)
		{	
			unsigned short port = mBinTcpSocket->remote_endpoint().port();
			std::string ip = mBinTcpSocket->remote_endpoint().address().to_string();

			this->InitMember(ip, port);
			this->mDispatchManager = manager;
			mCurrentSatte = SessionState::Normal;
		}
	}

	TcpClientSession::TcpClientSession(SessionManager * manager, std::string name, std::string ip, unsigned short port)
		: mAsioContext(manager->GetAsioContext()), mCurrentSatte(None)
	{		
		this->mIsContent = true;
		this->mSessionName = name;
		this->InitMember(ip, port);
		this->mDispatchManager = manager;
		this->mBinTcpSocket = std::make_shared<AsioTcpSocket>(mAsioContext);
	}

	void TcpClientSession::InitMember(const std::string & ip, unsigned short port)
	{
		this->mIp = ip;
		this->mPort = port;
		this->mConnectCount = 0;	
		mStartTime = TimeHelper::GetSecTimeStamp();
		

		this->mRecvBufferSize = 1024;
		this->mRecvMsgBuffer = new char[this->mRecvBufferSize];
		this->mAdress = this->mIp + ":" + std::to_string(this->mPort);
	}

	TcpClientSession::~TcpClientSession()
	{
		delete[]this->mRecvMsgBuffer;
	}

	bool TcpClientSession::IsActive()
	{
		return this->mCurrentSatte == Normal;
	}

	bool TcpClientSession::StartConnect()
	{
		if (this->IsActive() || this->mIsContent == false)
		{
			return false;
		}
		try
		{
			this->mConnectCount++;
			mCurrentSatte = SessionState::Connect;
			this->mStartTime = TimeHelper::GetSecTimeStamp();
			asio::ip::tcp::endpoint ep(asio::ip::address::from_string(mIp), mPort);
			this->mBinTcpSocket->async_connect(ep, [this](const asio::error_code & code)
			{
				if (code)
				{
					this->CloseSocket();
					SayNoDebugWarning("Connect " << this->GetSessionName() 
						<< " fail count = " << this->mConnectCount << " error : "<< code.message());
					return;
				}
				this->mConnectCount = 0;
				mCurrentSatte = SessionState::Normal;
				this->mDispatchManager->AddNewSession(shared_from_this());
			});
			return true;
		}
		catch (...)
		{
			SayNoDebugFatal("ip or port error" << this->mAdress);
			mCurrentSatte = SessionState::ConnectError;
		}
		return false;
	}

	void TcpClientSession::CloseSocket()
	{	
		if (this->mBinTcpSocket != nullptr && this->mBinTcpSocket->is_open())
		{
			asio::error_code closeCode;
			this->mBinTcpSocket->shutdown(asio::socket_base::shutdown_send, closeCode);
			this->mBinTcpSocket->shutdown(asio::socket_base::shutdown_receive, closeCode);
			this->mBinTcpSocket->close(closeCode);
		}
		this->mCurrentSatte = SessionState::ConnectError;
		this->mDispatchManager->AddErrorSession(shared_from_this());
	}

	bool TcpClientSession::SendPackage(const std::string & message)
	{
		const char * str = message.c_str();
		const size_t size = message.size();
		return this->SendPackage(str, size);
	}

	bool TcpClientSession::SendPackage(const char * message, const size_t size)
	{
		if (message == nullptr || size == 0)
		{
			return false;
		}
		if (!this->mBinTcpSocket || !this->mBinTcpSocket->is_open())
		{
			return false;
		}
		mBinTcpSocket->async_send(asio::buffer(message, size),
			[this](const asio::error_code & error_code, std::size_t size)
		{
			if (error_code)
			{
				this->CloseSocket();
			}		
		});
		return true;
	}

	void TcpClientSession::StartReceiveMsg()
	{
		this->mBinTcpSocket->async_read_some(asio::buffer(this->mRecvMsgBuffer, sizeof(unsigned int)),
			[this](const asio::error_code & error_code, const std::size_t t)
		{
			if (error_code)
			{
				this->CloseSocket();
				return;
			}
			size_t packageSize = 0;
			memcpy(&packageSize, this->mRecvMsgBuffer, t);
			this->ReadMessageBody(packageSize);
		});
	}

	void TcpClientSession::ReadMessageBody(const  size_t allSize)
	{
		char * nMessageBuffer = this->mRecvMsgBuffer;
		if (allSize > this->mRecvBufferSize)
		{
			nMessageBuffer = new char[allSize];
			if (nMessageBuffer == nullptr)
			{
				return;
			}
		}

		this->mBinTcpSocket->async_read_some(asio::buffer(nMessageBuffer, allSize),
			[this, nMessageBuffer](const asio::error_code & error_code, const std::size_t messageSize)
		{						
			if (error_code)
			{
				this->CloseSocket();
				return;
			}
			else
			{
				mAsioContext.post(std::bind(&TcpClientSession::StartReceiveMsg, this));
				this->mDispatchManager->AddRecvMessage(shared_from_this(), nMessageBuffer, messageSize);
			}
			if (nMessageBuffer != this->mRecvMsgBuffer)
			{
				delete []nMessageBuffer;
			}
		});
	}
}