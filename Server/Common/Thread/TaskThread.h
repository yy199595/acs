#pragma once
#include<mutex>
#include<queue>
#include"ThreadTaskAction.h"
#include<condition_variable>
#include<Other/DoubleBufferQueue.h>
namespace SoEasy
{
	enum ThreadState
	{
		Idle,
		Running,
	};
	class ThreadTaskManager;
	class TaskThread
	{
	public:
		TaskThread(ThreadTaskManager * taskManaer);
	public:
		void WaitToNextWake();
		long long GetThreadId() { return mThreadId; }
		void AddTaskAction(SharedThreadTask taskAction);
		ThreadState GetTaskState() { return this->mTaskState; }
		bool IsRunning() { return this->mTaskState == Running; }
	private:
		void Run();
	private:
		long long mThreadId;
		std::mutex mThreadLock;
		ThreadState mTaskState;
		std::thread * mBindThread;
		ThreadTaskManager * mTaskManager;
		std::condition_variable mThreadVarible;
		DoubleBufferQueue<SharedThreadTask> mWaitInvokeTask;
	};
}