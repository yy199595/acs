#pragma once
#include<Define/CommonTypeDef.h>
#include<Thread/TaskThread.h>
namespace Sentry
{

    struct ListenConfig
    {
    public:
        int Count = 0;
        std::string Ip;
        std::string Name;
        std::string Handler;
        unsigned short Port = 0;
    };

	class ISocketHandler;
	class NetworkListener
	{
	public:
		NetworkListener(NetWorkThread * thread, ListenConfig & config);
		~NetworkListener();
	public:
		bool StartListen(ISocketHandler * handler);
		bool IsOpen() { return this->mBindAcceptor->is_open(); }
        const ListenConfig & GetConfig() { return this->mConfig;}
	private:
		void ListenConnect();
		void OnConnectHandler(const asio::error_code & err);
	private:
        bool mIsListen;
        unsigned int mCorId;
        ListenConfig mConfig;
		NetWorkThread * mTaskThread;
		AsioTcpAcceptor *mBindAcceptor;	
		ISocketHandler * mSessionHandler;
		class SessionBase * mSessionSocket;
		class MainTaskScheduler & mTaskScheduler;
	};
}