#pragma once
#include<mutex>
#include<queue>
#include<thread>
#include"ThreadTaskAction.h"
#include<condition_variable>
#include<Other/DoubleBufferQueue.h>
namespace Sentry
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
		TaskThread(ThreadTaskManager * taskManaer,int index);
	public:
		void WaitToNextWake();
		void AddTaskAction(SharedThreadTask taskAction);
		ThreadState GetTaskState() { return this->mTaskState; }
		bool IsRunning() { return this->mTaskState == Running; }
	private:
		void Run();
	private:
		int mThreadIndex;
		std::mutex mThreadLock;
		ThreadState mTaskState;
		std::thread * mBindThread;
		ThreadTaskManager * mTaskManager;
		std::condition_variable mThreadVarible;
		DoubleBufferQueue<SharedThreadTask> mWaitInvokeTask;
	};
}