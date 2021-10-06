#pragma once

#include "TaskProxy.h"
#include <Other/DoubleBufferQueue.h>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

namespace Sentry
{
    enum ThreadState
    {
        Idle,
        Run,
    };

    class TaskComponent;

    class TaskThread
    {
    public:
        TaskThread(TaskComponent *taskComponent);

    public:
        void AddTask(TaskProxy * task);

        ThreadState GetTaskState() { return this->mTaskState; }

		const std::thread::id GetId() { return this->mThreadId; }
        bool IsRunning() { return this->mTaskState == ThreadState::Run; }

    private:
        void Run();

    private:
        bool mIsStop;
        std::mutex mThreadLock;
        ThreadState mTaskState;
        std::thread * mBindThread;
        TaskComponent *mTaskManager;
		std::thread::id mThreadId;
		std::queue<unsigned int> mFinishTasks;
        std::condition_variable mThreadVariable;
        DoubleBufferQueue<TaskProxy *> mWaitInvokeTask;
    };
}// namespace Sentry