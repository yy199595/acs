#pragma once
#include"Async/TaskSource.h"
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
        std::string mAddress;
        unsigned short Port = 0;
    };

	class ISocketListen;
	class NetworkListener
	{
	public:
		NetworkListener(IAsioThread & thread, ListenConfig & config);
		~NetworkListener();
	public:
        bool IsOpen() { return this->mBindAcceptor->is_open(); }
        const ListenConfig & GetConfig() const { return this->mConfig;}
		std::shared_ptr<TaskSource<bool>> StartListen(ISocketListen * handler);
	private:
		void ListenConnect();
        void InitListener(std::shared_ptr<TaskSource<bool>> taskSource);
    private:
        bool mIsListen;
        unsigned int mCorId;
        ListenConfig mConfig;
        IAsioThread & mTaskThread;
		AsioTcpAcceptor *mBindAcceptor;	
		ISocketListen * mListenHandler;
		ThreadPoolComponent * mTaskComponent;
		class MainTaskScheduler & mTaskScheduler;
	};
}