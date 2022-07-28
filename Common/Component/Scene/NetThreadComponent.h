#pragma once

#include"Component/Component.h"
#include"Util/NumberBuilder.h"
namespace Sentry
{
	class IThreadTask;

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
	 public:
#ifndef ONLY_MAIN_THREAD
        asio::io_service & AllocateNetThread(int index = 0);
#endif
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