#pragma once

#include"Component/Component.h"
#include "Thread/TaskThread.h"
#include "Other/MultiThreadQueue.h"
#include "Util/NumberBuilder.h"
namespace Sentry
{
	class TaskProxy;

	class ThreadPoolComponent : public Component
	{
	 public:
		ThreadPoolComponent() = default;
		~ThreadPoolComponent() final = default;

	 public:
		void Awake() final;

		bool LateAwake() final;

		void OnDestory() final;
	 public:

		bool StartTask(TaskProxy* taskAction);

		void GetAllThread(std::vector<const IThread*>& threads);

		const std::vector<TaskThread*>& GetThreads()
		{
			return this->mThreadArray;
		}

	 public:
#ifndef ONLY_MAIN_THREAD
		IAsioThread & AllocateNetThread();
#endif
	 private:
		size_t mIndex;
		std::mutex mLock;
		std::thread* mMonitorThread;
		std::vector<TaskThread*> mThreadArray;
#ifndef ONLY_MAIN_THREAD
		std::vector<NetWorkThread *> mNetThreads;
#endif
		NumberBuilder<unsigned int> mTaskNumberPool;
	};
}