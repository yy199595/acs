#pragma once
#include<list>
#include<string>
#include"Tcp/Asio.h"
#include"Component/Component.h"

namespace Sentry
{
    class AsioThread : protected std::thread
    {
    public:
        AsioThread();
    public:
        void Run() { this->detach(); }
        Asio::Context & Context() { return *mContext; }
    private:
        void Update();
    private:
        Asio::Context * mContext;
        Asio::ContextWork * mWork;
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