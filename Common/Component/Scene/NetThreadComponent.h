#pragma once

#include"Component/Component.h"
#include"Util/NumberBuilder.h"
namespace Sentry
{
	class NetThreadComponent : public Component
	{
	 public:
		NetThreadComponent() = default;
		~NetThreadComponent() final = default;

	 public:
		void Awake() final;
		bool LateAwake() final;
		void OnDestory() final;
    public:
        std::shared_ptr<SocketProxy> CreateSocket();
        std::shared_ptr<SocketProxy> CreateSocket(const std::string & ip, unsigned short port);
    private:
		size_t mIndex;
		std::thread* mMonitorThread;
#ifndef ONLY_MAIN_THREAD
        std::vector<std::thread *> mThreads;
        std::vector<asio::io_service *> mNetThreads;
        std::vector<asio::io_service::work *> mNetThreadWorks;
#endif
	};
}