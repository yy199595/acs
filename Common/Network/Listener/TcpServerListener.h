#pragma once
#include"Async/TaskSource.h"
#include<Define/CommonTypeDef.h>
#include<Thread/TaskThread.h>
namespace Sentry
{
	struct ListenConfig
	{
	 public:
		std::string Ip;
		std::string Name;
		std::string Token;
		std::string Handler;
		std::string Address;
		unsigned short Port = 0;
	};

	class ISocketListen;
    class TcpServerComponent;
	class TcpServerListener
	{
	 public:
		TcpServerListener(IAsioThread& thread);
		~TcpServerListener();
	 public:
		bool StartListen(const ListenConfig* config, TcpServerComponent * component);
	 private:
		void ListenConnect();
	 private:
		int mCount;
		int mErrorCount;
		IAsioThread& mTaskThread;
		const ListenConfig * mConfig;
        AsioTcpAcceptor* mBindAcceptor;
        TcpServerComponent * mTcpComponent;
    };
}