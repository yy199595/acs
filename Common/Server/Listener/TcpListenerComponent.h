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
        bool StartListen(const char * name);
        const ListenConfig & GetListenConfig() const { return *this->mConfig; }
        const std::string & GetListenAddress() const { return this->mConfig->Address; }
    protected:
        void ListenConnect();
        virtual bool OnListen(std::shared_ptr<SocketProxy> socket) = 0;
    private:
		int mCount;
		int mErrorCount;
		const ListenConfig * mConfig;
        NetThreadComponent * mNetComponent;
        asio::ip::tcp::acceptor * mBindAcceptor;
    };
}