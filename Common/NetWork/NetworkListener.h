#pragma once
#include<Define/CommonTypeDef.h>
#include<Thread/TaskThread.h>
namespace Sentry
{

    struct ListenConfig
    {
    public:
        int Count;
        std::string Name;
        std::string Handler;
        unsigned short Port;
    };

	class ISocketHandler;
	class NetworkListener
	{
	public:
		NetworkListener(NetWorkThread * thread, ListenConfig & config);
	public:
		void StartListen(ISocketHandler * handler);
		bool IsOpen() { return this->mBindAcceptor->is_open(); }
        const ListenConfig & GetConfig() { return this->mConfig;}
	private:
		void ListenConnect();
		void OnConnectHandler(const asio::error_code & err);
	private:
        ListenConfig mConfig;
		NetWorkThread * mTaskThread;
		AsioTcpAcceptor *mBindAcceptor;	
		ISocketHandler * mSessionHandler;
		class SessionBase * mSessionSocket;
		class MainTaskScheduler & mTaskScheduler;
	};
}