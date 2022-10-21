#pragma once
#include<vector>
#include"Tcp/Asio.h"
#include"Component/Component.h"
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
        std::vector<std::thread *> mThreads;
        std::vector<Asio::Context *> mNetThreads;
        std::vector<Asio::ContextWork *> mNetThreadWorks;
#endif
	};
}