#include"TcpClientSession.h"
#include<Core/Applocation.h>
#include<Util/StringHelper.h>
#include<Manager/SessionManager.h>
namespace SoEasy
{
	TcpClientSession::TcpClientSession(SessionManager * manager, SharedTcpSocket socket)
		:mAsioContext(manager->GetAsioContext()), mCurrentSatte(Session_None)
	{
		this->mIsContent = false;
		this->mBinTcpSocket = socket;
		
		if (this->mBinTcpSocket != nullptr)
		{
			this->mDispatchManager = manager;
			mCurrentSatte = SessionState::Session_Normal;
			this->mSocketEndPoint = socket->remote_endpoint();
			this->InitMember(this->mSocketEndPoint.address().to_string(), this->mSocketEndPoint.port());
			this->mSocketId = (long long)this->mSocketEndPoint.address().to_v4().to_uint() << 32 | this->mPort;
		}
	}

	TcpClientSession::TcpClientSession(SessionManager * manager, std::string name, std::string ip, unsigned short port)
		: mAsioContext(manager->GetAsioContext()), mCurrentSatte(Session_None)
	{		
		this->mIsContent = true;
		this->mSessionName = name;
		this->InitMember(ip, port);
		this->mDispatchManager = manager;
		this->mBinTcpSocket = std::make_shared<AsioTcpSocket>(mAsioContext);
		this->mSocketEndPoint = asio::ip::tcp::endpoint(asio::ip::address::from_string(mIp), mPort);
		this->mSocketId = (long long)this->mSocketEndPoint.address().to_v4().to_uint() << 32 | this->mPort;
	}

	void TcpClientSession::InitMember(const std::string & ip, unsigned short port)
	{
		this->mIp = ip;
		this->mPort = port;
		this->mConnectCount = 0;
		this->mRecvBufferSize = 1024;
		mStartTime = TimeHelper::GetSecTimeStamp();
		this->mRecvMsgBuffer = new char[this->mRecvBufferSize];
		this->mAdress = this->mIp + ":" + std::to_string(this->mPort);
		
		this->mRecvAction = BIND_THIS_ACTION_0(TcpClientSession::Receive);
		this->mCloseAction = BIND_THIS_ACTION_0(TcpClientSession::CloseSocket);
		this->mConnectAction = BIND_THIS_ACTION_0(TcpClientSession::Connect);
	}

	TcpClientSession::~TcpClientSession()
	{
		delete[]this->mRecvMsgBuffer;
	}

	bool TcpClientSession::IsActive()
	{
		return this->mCurrentSatte == Session_OK;
	}

	bool TcpClientSession::SendPackage(std::shared_ptr<std::string> message)
	{
		if (message == nullptr || message->size() == 0)
		{
			return false;
		}
		if (!this->mBinTcpSocket || !this->mBinTcpSocket->is_open())
		{
			return false;
		}
		this->mAsioContext.post([this, message]()
		{
			mBinTcpSocket->async_send(asio::buffer(message->c_str(), message->size()),
				[this](const asio::error_code & error_code, std::size_t size)
			{
				if (error_code)
				{
					this->CloseSocket();
				}
			});
		});	
		return true;
	}

	void TcpClientSession::StartClose()
	{
		this->mCurrentSatte = Session_Close;
		this->mAsioContext.post(this->mCloseAction);
	}

	bool TcpClientSession::StartConnect()
	{
		if (this->IsActive() || this->mIsContent == false)
		{
			return false;
		}
		if (mCurrentSatte == SessionState::Session_Connect)
		{
			return false;
		}
		this->mConnectCount++;
		mCurrentSatte = SessionState::Session_Connect;
		this->mStartTime = TimeHelper::GetSecTimeStamp();
		this->mAsioContext.post(BIND_THIS_ACTION_0(TcpClientSession::Connect));
		SayNoDebugLog(this->GetSessionName() << " start connect " << this->mAdress);
		return true;
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
		this->mCurrentSatte = SessionState::Session_Error;
		this->mDispatchManager->AddErrorSession(shared_from_this());
	}

	bool TcpClientSession::StartReceiveMsg()
	{
		if (this->mCurrentSatte != Session_Normal)
		{
			return false;
		}
		this->mAsioContext.post(this->mRecvAction);
		return true;
	}

	void TcpClientSession::Connect()
	{
		this->mBinTcpSocket->async_connect(this->mSocketEndPoint, [this](const asio::error_code & code)
		{
			if (code)
			{
				this->CloseSocket();
				SayNoDebugWarning("Connect " << this->GetSessionName() 
					<< " fail count = " << this->mConnectCount << " error : " << code.message());
				return;
			}
			this->mConnectCount = 0;
			mCurrentSatte = Session_Normal;
			this->mDispatchManager->AddNewSession(shared_from_this());
		});
	}

	void TcpClientSession::Receive()
	{
		this->mCurrentSatte = Session_OK;
		this->mBinTcpSocket->async_read_some(asio::buffer(this->mRecvMsgBuffer, sizeof(unsigned int)),
			[this](const asio::error_code & error_code, const std::size_t t)
		{
			if (error_code)
			{
				SayNoDebugError(error_code.message());
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
			}
			else
			{
				if (!this->mDispatchManager->AddRecvMessage(shared_from_this(), nMessageBuffer, messageSize))
				{
					this->CloseSocket();
					return;
				}
				mAsioContext.post(this->mRecvAction);
			}
			if (nMessageBuffer != this->mRecvMsgBuffer)
			{
				delete []nMessageBuffer;
			}
		});
	}
}