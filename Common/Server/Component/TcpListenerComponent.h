#pragma once
#include"asio/ip/tcp.hpp"
#include"Core/Component/Component.h"
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
        unsigned short GetListenPort() const { return this->mListenPort;}
    protected:
        void ListenConnect();
        virtual void OnStopListen() { };
        virtual void OnListen(std::shared_ptr<SocketProxy> socket) = 0;
    private:
        size_t mListenCount;
        unsigned short mListenPort;
        class ThreadComponent * mThreadComponent;
        std::unique_ptr<asio::ip::tcp::acceptor> mBindAcceptor;
    };
}