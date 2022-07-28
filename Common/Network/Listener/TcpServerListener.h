#pragma once
#include"Async/TaskSource.h"
#include"Define/CommonTypeDef.h"
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
		TcpServerListener(const ListenConfig * config);
		~TcpServerListener();
	 public:
		bool StartListen( asio::io_service& io, TcpServerComponent * component);
	 private:
		void ListenConnect();
	 private:
		int mCount;
		int mErrorCount;
		const ListenConfig * mConfig;
        AsioTcpAcceptor* mBindAcceptor;
        TcpServerComponent * mTcpComponent;
    };
}