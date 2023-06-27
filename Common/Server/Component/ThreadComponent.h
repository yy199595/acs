#pragma once
#include<list>
#include<string>
#include"Network/Tcp/SocketProxy.h"
#include"Entity/Component/Component.h"

namespace Tendo
{
    class AsioThread
    {
    public:
        AsioThread();
    public:
        void Run();
        Asio::Context & Context() { return *mContext; }
    private:
        std::thread* mThread;
        Asio::Context * mContext;
    };
}

namespace Tendo
{
	class ThreadComponent : public Component, public IDestroy
	{
	 public:
		ThreadComponent() = default;
	 public:
		bool Awake() final;
		void OnDestroy() final;
    public:
        Asio::Context& GetContext();
        std::shared_ptr<Tcp::SocketProxy> CreateSocket();
		std::shared_ptr<Tcp::SocketProxy> CreateSocket(const std::string & ip, unsigned short port);
    private:
#ifndef ONLY_MAIN_THREAD
        std::mutex mMutex;
        std::list<AsioThread *> mNetThreads;
#endif
	};
}