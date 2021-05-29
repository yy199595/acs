#pragma once
#include<mutex>
#include<queue>
#include"ThreadTaskAction.h"
#include<condition_variable>
namespace SoEasy
{
	enum ThreadState
	{
		Idle,
		Running,
	};
	class ThreadPool;
	class TaskThread
	{
	public:
		TaskThread();
	public:
		void WaitToNextWake();
		long long GetThreadId() { return mThreadId; }
		bool AddTaskAction(SharedThreadTask taskAction);
		ThreadState GetTaskState() { return this->mTaskState; }
		bool IsRunning() { return this->mTaskState == Running; }
	private:
		void Run();
	private:
		long long mThreadId;
		std::mutex mThreadLock;
		ThreadState mTaskState;
		std::thread * mBindThread;
		std::condition_variable mThreadVarible;
		std::queue<SharedThreadTask> mWaitInvokeTask;
	};
}