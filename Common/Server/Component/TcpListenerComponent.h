#pragma once
#include"Source/TaskSource.h"
#include"Config/ServerConfig.h"
#include"Component/Component.h"
namespace Sentry
{
    class NetThreadComponent;

    class TcpListenerComponent : public Component
	{
	 public:
		TcpListenerComponent();
		~TcpListenerComponent();
	 public:
        bool StopListen();
        bool StartListen(const char * name);
    protected:
        void ListenConnect();
        virtual void OnStopListen() { };
        virtual bool OnListen(std::shared_ptr<SocketProxy> socket) = 0;
    private:
		int mCount;
        bool mIsClose;
        int mErrorCount;     
        NetThreadComponent * mNetComponent;
        asio::ip::tcp::acceptor * mBindAcceptor;
    };
}