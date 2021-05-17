#include"TcpSessionListener.h"
#include<Core/Applocation.h>
#include<Manager/SessionManager.h>
#include<NetWork/TcpClientSession.h>
namespace SoEasy
{
	TcpSessionListener::TcpSessionListener(SessionManager * manager, const unsigned short port)
		: mAsioContext(manager->GetAsioContext())
	{
		this->mListenerPort = port;
		this->mBindAcceptor = nullptr;
		this->mDispatchManager = manager;
	}
	bool TcpSessionListener::InitListener()
	{
		if (this->mBindAcceptor == nullptr)
		{
			try
			{
				AsioTcpEndPoint tBindPoint(asio::ip::tcp::v4(), this->mListenerPort);
				this->mBindAcceptor = new AsioTcpAcceptor(mAsioContext, tBindPoint);
				this->mBindAcceptor->listen();
				this->mListenAction = BIND_THIS_ACTION_0(TcpSessionListener::Listen);
			}
			catch (const asio::system_error& e)
			{
				SayNoDebugError("start server fail " << e.what());
				return false;
			}
			return true;
		}
		return false;
	}

	void TcpSessionListener::Listen()
	{
		SharedTcpSocket tcpSocket = std::make_shared<AsioTcpSocket>(mAsioContext);
		this->mBindAcceptor->async_accept(*tcpSocket, [this, tcpSocket](const asio::error_code & code)
		{
			if (!code)
			{
				unsigned short port = tcpSocket->remote_endpoint().port();
				std::string ip = tcpSocket->remote_endpoint().address().to_string();
				this->mDispatchManager->CreateTcpSession(tcpSocket);
			}
			this->mAsioContext.post(mListenAction);
		});
	}

	shared_ptr<TcpClientSession> TcpSessionListener::CreateTcpSession(SharedTcpSocket socket)
	{
		return std::make_shared<TcpClientSession>(this->mDispatchManager, socket);
	}

	void TcpSessionListener::StartAcceptConnect()
	{
		this->mAsioContext.post(mListenAction);
	}

	TcpSessionListener::~TcpSessionListener()
	{
		DeletePtr(this->mBindAcceptor);
	}

}