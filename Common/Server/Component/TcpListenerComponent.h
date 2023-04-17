#pragma once
#include"Network/Tcp/SocketProxy.h"
#include"Entity/Component/Component.h"
namespace Tendo
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
        virtual void OnListen(std::shared_ptr<Tcp::SocketProxy> socket) = 0;
    private:
		std::string mNet;
        size_t mListenCount;
        unsigned short mListenPort;
		std::mutex mMutex;
		std::condition_variable mVal;
        class ThreadComponent * mThreadComponent;
        std::unique_ptr<asio::ip::tcp::acceptor> mBindAcceptor;
    };
}