﻿#pragma once
#include<list>
#include<mutex>
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
        Asio::Context & Context() const { return *mContext; }
    private:
        void Update();
    private:
        std::unique_ptr<Asio::Context> mContext;
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
        std::mutex mMutex;
#ifndef ONLY_MAIN_THREAD
        std::list<AsioThread *> mNetThreads;
#endif
	};
}