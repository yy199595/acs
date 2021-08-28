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

    class SceneTaskComponent;

    class TaskThread
    {
    public:
        TaskThread(SceneTaskComponent *taskManaer, int index);

    public:
        void WaitToNextWake();

        void AddTask(TaskProxy * task);

        ThreadState GetTaskState() { return this->mTaskState; }

		std::thread::id & GetId() { return this->mThreadId; }
        bool IsRunning() { return this->mTaskState == ThreadState::Run; }

    private:
        void Run();

    private:
        int mThreadIndex;
		
        std::mutex mThreadLock;
        ThreadState mTaskState;
        std::thread *mBindThread;
        SceneTaskComponent *mTaskManager;
		std::thread::id mThreadId;
		std::queue<unsigned int> mFinishTasks;
        std::condition_variable mThreadVarible;
        DoubleBufferQueue<TaskProxy *> mWaitInvokeTask;
    };
}// namespace Sentry