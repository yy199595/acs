#pragma once
#include<Object/Object.h>
namespace SoEasy
{
	class SessionManager;
	class TcpSessionListener
	{
	public:
		TcpSessionListener(SessionManager * manager, const unsigned short port);
		~TcpSessionListener();
	public:
		bool InitListener();
		void StartAcceptConnect();
		unsigned short GetListenPort() { return this->mListenerPort; }
	private:
		shared_ptr<TcpClientSession> CreateTcpSession(SharedTcpSocket socket);
	private:		
		AsioContext & mAsioContext;	
		unsigned short mListenerPort;
		AsioTcpAcceptor * mBindAcceptor;
		SessionManager * mDispatchManager;
	};
}