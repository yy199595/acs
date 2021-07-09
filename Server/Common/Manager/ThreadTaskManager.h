#pragma once

#include"Manager.h"

namespace SoEasy
{
	class ThreadTaskManager : public Manager, public ISystemUpdate
	{
	public:
		ThreadTaskManager() { }
		~ThreadTaskManager() { }
	public:
		void OnSystemUpdate() final;
	private:
		DoubleBufferQueue<long long> mFinishTaskQueue;  //在其他线程完成的任务存储
		std::queue<shared_ptr<ThreadTaskAction>> mWaitDispatchTasks; //等待派发的任务
		std::unordered_map<long long, shared_ptr<ThreadTaskAction>> mThreadTaskMap;
	};
}