#pragma once
#include<list>
#include<string>
#include"Network/Tcp/Asio.h"
#include"Core/Component/Component.h"

namespace Sentry
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

namespace Sentry
{
	class ThreadComponent : public Component
	{
	 public:
		ThreadComponent() = default;
	 public:
		bool Awake() final;
		bool LateAwake() final;
		void OnDestroy() final;
    public:
        Asio::Context& GetContext();
        std::shared_ptr<SocketProxy> CreateSocket();
		std::shared_ptr<SocketProxy> CreateSocket(const std::string & address);
		std::shared_ptr<SocketProxy> CreateSocket(const std::string & ip, unsigned short port);
    private:
#ifndef ONLY_MAIN_THREAD
        std::list<AsioThread *> mNetThreads;
#endif
	};
}