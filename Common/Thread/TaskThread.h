#pragma once

#include "TaskProxy.h"
#include <Other/DoubleBufferQueue.h>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <Define/CommonDef.h>
namespace Sentry
{
    class TaskComponent;
    enum ThreadState
    {
        Idle,
        Run,
    };

    class IThread
    {
    public:
        IThread(TaskComponent *taskComponent);

        virtual ~IThread()
        {}

    public:
        void Stop()
        { this->mIsClose = true; }

        virtual void AddTask(TaskProxy *task) = 0;

        const std::thread::id GetThreadId()
        { return this->mThreadId; }

    protected:
        virtual void Update() { };

    private:
        void Run();

    private:
        bool mIsClose;
        std::thread *mThread;
        std::thread::id mThreadId;
    protected:
        TaskComponent *mTaskComponent;
    };

    class TaskThread : public IThread
    {
    public:
        TaskThread(TaskComponent * taskComponent);

    public:
        void AddTask(TaskProxy * task) final;
        ThreadState GetTaskState() { return this->mTaskState; }
        bool IsRunning() { return this->mTaskState == ThreadState::Run; }

    protected:
        void Update() final;
    private:
        std::mutex mThreadLock;
        ThreadState mTaskState;
		std::queue<unsigned int> mFinishTasks;
        std::condition_variable mThreadVariable;
        DoubleBufferQueue<TaskProxy *> mWaitInvokeTask;
    };

    class NetWorkThread : public IThread
    {
    public:
        NetWorkThread(TaskComponent * taskComponent, class MethodProxy * method = nullptr);
    public:
        void AddTask(TaskProxy * task) final;
    protected:
        void Update() final;
    private:
        class MethodProxy * mMethodProxy;
        std::queue<unsigned int> mFinishTasks;
        DoubleBufferQueue<TaskProxy *> mWaitInvokeTask;
    };
}// namespace Sentry