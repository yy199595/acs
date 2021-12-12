#pragma once
#include<Define/CommonTypeDef.h>
#include<Thread/TaskThread.h>
namespace GameKeeper
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

	class ISocketListen;
	class NetworkListener
	{
	public:
		NetworkListener(NetWorkThread & thread, ListenConfig & config);
		~NetworkListener();
	public:
		bool StartListen(ISocketListen * handler);
		bool IsOpen() { return this->mBindAcceptor->is_open(); }
        const ListenConfig & GetConfig() const { return this->mConfig;}
	private:
        void InitListener();
		void ListenConnect();
	private:
        bool mIsListen;
        unsigned int mCorId;
        ListenConfig mConfig;
		NetWorkThread & mTaskThread;
		AsioTcpAcceptor *mBindAcceptor;	
		ISocketListen * mListenHandler;
		TaskPoolComponent * mTaskComponent;
		class MainTaskScheduler & mTaskScheduler;
	};
}