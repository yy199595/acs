#pragma once
#include<vector>
#include"Tcp/Asio.h"
#include"Component/Component.h"

namespace Sentry
{
    class AsioThread
    {
    public:
        bool Run();
        Asio::Context & Context() const { return *mContext; }
    private:
        void Update();
    private:
        std::unique_ptr<std::thread> mThread;
        std::unique_ptr<Asio::Context> mContext;
    };
}

namespace Sentry
{
	class NetThreadComponent : public Component
	{
	 public:
		NetThreadComponent() = default;
	 public:
		bool Awake() final;
		bool LateAwake() final;
		void OnDestory() final;
    public:
        std::shared_ptr<SocketProxy> CreateSocket();
		std::shared_ptr<SocketProxy> CreateSocket(const std::string & address);
		std::shared_ptr<SocketProxy> CreateSocket(const std::string & ip, unsigned short port);
    private:
		size_t mIndex;
		std::thread* mMonitorThread;
#ifndef ONLY_MAIN_THREAD
        std::vector<std::unique_ptr<AsioThread>> mNetThreads;
#endif
	};
}