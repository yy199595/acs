#pragma once

#include "ThreadTaskAction.h"
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

    class ThreadTaskManager;

    class TaskThread
    {
    public:
        TaskThread(ThreadTaskManager *taskManaer, int index);

    public:
        void WaitToNextWake();

        void AddTaskAction(SharedThreadTask taskAction);

        ThreadState GetTaskState() { return this->mTaskState; }

        bool IsRunning() { return this->mTaskState == ThreadState::Run; }

    private:
        void Run();

    private:
        int mThreadIndex;
        std::mutex mThreadLock;
        ThreadState mTaskState;
        std::thread *mBindThread;
        ThreadTaskManager *mTaskManager;
        std::condition_variable mThreadVarible;
        DoubleBufferQueue<SharedThreadTask> mWaitInvokeTask;
    };
}// namespace Sentry