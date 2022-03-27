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
		NetworkListener(IAsioThread& thread, ListenConfig& config);
		~NetworkListener();
	 public:
		bool StartListen(ISocketListen* handler);
		bool IsOpen()
		{
			return this->mBindAcceptor->is_open();
		}
		const ListenConfig& GetConfig() const
		{
			return this->mConfig;
		}
	 private:
		void ListenConnect();
#ifndef ONLY_MAIN_THREAD
		void InitListener(std::shared_ptr<TaskSource<bool>> taskSource);
#endif
	 private:
		bool mIsListen;
		unsigned int mCorId;
		ListenConfig mConfig;
		IAsioThread& mTaskThread;
		AsioTcpAcceptor* mBindAcceptor;
		ISocketListen* mListenHandler;
		ThreadPoolComponent* mTaskComponent;
		class MainTaskScheduler& mTaskScheduler;
	};
}