#include"TcpClientSession.h"
#include<Core/Applocation.h>
#include<Util/StringHelper.h>
#include<Manager/NetSessionManager.h>
#include<NetWork/SocketEvent.h>
namespace SoEasy
{
	TcpClientSession::TcpClientSession(NetSessionManager * manager, SharedTcpSocket socket)
		:mAsioContext(manager->GetAsioCtx()), mCurrentSatte(Session_None)
	{
		this->mIsContent = false;
		this->mBinTcpSocket = socket;
		
		if (this->mBinTcpSocket != nullptr)
		{
			this->mDispatchManager = manager;
			mCurrentSatte = SessionState::Session_Normal;
			this->mSocketEndPoint = socket->remote_endpoint();
			this->InitMember(this->mSocketEndPoint.address().to_string(), this->mSocketEndPoint.port());
		}
	}

	TcpClientSession::TcpClientSession(NetSessionManager * manager, std::string name, std::string ip, unsigned short port)
		:mAsioContext(manager->GetAsioCtx()), mCurrentSatte(Session_None)
	{		
		this->mIsContent = true;
		this->mSessionName = name;
		this->InitMember(ip, port);
		this->mDispatchManager = manager;
		this->mSocketEndPoint = asio::ip::tcp::endpoint(asio::ip::address::from_string(mIp), mPort);
	}

	void TcpClientSession::InitMember(const std::string & ip, unsigned short port)
	{
		this->mIp = ip;
		this->mPort = port;
		this->mConnectCount = 0;
		this->mRecvBufferSize = 1024;
		this->mRecvMsgBuffer = new char[this->mRecvBufferSize];
		this->mAdress = this->mIp + ":" + std::to_string(this->mPort);
		this->mRecvAction = BIND_THIS_ACTION_0(TcpClientSession::StartReceiveMsg);
	}

	TcpClientSession::~TcpClientSession()
	{
		delete[]this->mRecvMsgBuffer;
	}

	bool TcpClientSession::IsActive()
	{
		return this->mCurrentSatte == Session_OK;
	}

	bool TcpClientSession::SendPackage(const shared_ptr<std::string> message)
	{
		if (message == nullptr || message->size() == 0)
		{
			return false;
		}
		if (!this->mBinTcpSocket || !this->mBinTcpSocket->is_open())
		{
			return false;
		}

		mBinTcpSocket->async_send(asio::buffer(message->c_str(), message->size()),
			[this, message](const asio::error_code & error_code, std::size_t size)
			{
				this->StartClose();
				const char * msg = message->c_str();
				this->mDispatchManager->OnSendMessageError(shared_from_this(), msg, size);				
			});
		return true;
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
		if (this->mBinTcpSocket == nullptr)
		{
			this->mBinTcpSocket = std::make_shared<AsioTcpSocket>(this->mAsioContext);
		}
		this->mBinTcpSocket->async_connect(this->mSocketEndPoint, [this](const asio::error_code & error_code)
			{
				if (error_code)
				{			
					this->StartClose();
					SayNoDebugWarning("Connect " << this->GetSessionName()
						<< " fail count = " << this->mConnectCount << " error : " << error_code.message());
					this->mDispatchManager->OnSessionError(shared_from_this(), Net2MainEventType::SocketConnectFail);
				}
				else
				{
					this->mConnectCount = 0;
					this->StartReceiveMsg();
					this->mDispatchManager->OnConnectSuccess(shared_from_this());
				}			
			});
		SayNoDebugLog(this->GetSessionName() << " start connect " << this->mAdress);
		return true;
	}

	void TcpClientSession::StartClose()
	{	
		if (this->mBinTcpSocket != nullptr && this->mBinTcpSocket->is_open())
		{
			asio::error_code closeCode;
			this->mBinTcpSocket->shutdown(asio::socket_base::shutdown_send, closeCode);
			this->mBinTcpSocket->shutdown(asio::socket_base::shutdown_receive, closeCode);
			this->mBinTcpSocket->close(closeCode);
		}
		this->mCurrentSatte = SessionState::Session_Error;
	}

	void TcpClientSession::StartReceiveMsg()
	{
		this->mCurrentSatte = Session_OK;
		this->mBinTcpSocket->async_read_some(asio::buffer(this->mRecvMsgBuffer, sizeof(unsigned int)),
			[this](const asio::error_code & error_code, const std::size_t t)
			{
				if (error_code)
				{
					this->StartClose();
					SayNoDebugError(error_code.message());
					this->mDispatchManager->OnSessionError(shared_from_this(), SocketReceiveFail);
				}
				else
				{
					size_t packageSize = 0;
					memcpy(&packageSize, this->mRecvMsgBuffer, t);
					this->ReadMessageBody(packageSize);
				}
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
				this->StartClose();
				this->mDispatchManager->OnSessionError(shared_from_this(), SocketReceiveFail);
				return;
			}
		}

		this->mBinTcpSocket->async_read_some(asio::buffer(nMessageBuffer, allSize),
			[this, nMessageBuffer](const asio::error_code & error_code, const std::size_t messageSize)
			{
				if (error_code)
				{
					this->StartClose();
					SayNoDebugError(error_code.message());
					this->mDispatchManager->OnSessionError(shared_from_this(), SocketReceiveFail);
				}
				else
				{
					if (!this->mDispatchManager->OnRecvMessage(shared_from_this(), nMessageBuffer, messageSize))
					{
						this->StartClose();
					}
					else
					{
						this->mAsioContext.post(this->mRecvAction);
					}					
				}
				if (nMessageBuffer != this->mRecvMsgBuffer)
				{
					delete[]nMessageBuffer;
				}
			});
	}
}