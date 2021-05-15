#pragma once
#include<mutex>
#include<queue>
#include"ThreadTaskAction.h"
#include<condition_variable>
namespace SoEasy
{
	class ThreadPool;
	class TaskThread
	{
	public:
		TaskThread();
	public:
		void WaitToNextWake();
		long long GetThreadId() { return mThreadId; }
		void AddTaskAction(SharedThreadTask taskAction);
		size_t GetTaskSize() const { return this->mWaitInvokeTask.size() + this->mTaskBuffer.size(); }
	private:
		void Run();
	private:
		long long mThreadId;
		std::mutex mThreadLock;
		std::thread * mBindThread;
		std::condition_variable mThreadVarible;
		std::queue<SharedThreadTask> mTaskBuffer;
		std::queue<SharedThreadTask> mWaitInvokeTask;
	};
}