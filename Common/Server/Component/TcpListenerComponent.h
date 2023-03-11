#pragma once
#include"asio/ip/tcp.hpp"
#include"Component/Component.h"
namespace Sentry
{
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
        virtual void OnListen(std::shared_ptr<SocketProxy> socket) = 0;
    private:
        size_t mListenCount;
        asio::ip::tcp::acceptor * mBindAcceptor;
        class ThreadComponent * mThreadComponent;
    };
}