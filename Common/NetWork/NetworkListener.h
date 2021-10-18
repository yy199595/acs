#pragma once
#include<Define/CommonTypeDef.h>
#include<Thread/TaskThread.h>
namespace Sentry
{
	class ISocketHandler;
	class NetworkListener
	{
	public:
		NetworkListener(const std::string & name, NetWorkThread * thread, unsigned short port, int maxCount);
	public:
		void StartListen(ISocketHandler * handler);
		const std::string & GetName() { return this->mName; }
		const unsigned short GetPort() { return this->mPort; }
		bool IsOpen() { return this->mBindAcceptor->is_open(); }
	private:
		void ListenConnect();
		void OnConnectHandler(const asio::error_code & err);
	private:
		
		const int mMaxCount;
		const std::string mName;
		const unsigned short mPort;	
		NetWorkThread * mTaskThread;
		AsioTcpAcceptor *mBindAcceptor;	
		ISocketHandler * mSessionHandler;
		class SessionBase * mSessionSocket;
		class MainTaskScheduler & mTaskScheduler;
	};
}