#include "TaskThread.h"
#include <Scene/TaskComponent.h>
#include <functional>
#include <Method/MethodProxy.h>
using namespace std::chrono;

namespace Sentry
{
    IThread::IThread(TaskComponent * task)
    {
        this->mTaskComponent = task;
        this->mThread = new std::thread(std::bind(&IThread::Run, this));
        this->mThreadId = this->mThread->get_id();
        this->mThread->detach();
        this->mIsClose = false;
    }

    void IThread::Run()
    {
        std::chrono::milliseconds time(1);
        while(!this->mIsClose)
        {
            this->Update();
            std::this_thread::sleep_for(time);
        }
    }
}

namespace Sentry
{
    TaskThread::TaskThread(TaskComponent * taskComponent)
        : IThread(taskComponent)
    {
        this->mTaskState = Idle;
    }

    void TaskThread::AddTask(TaskProxy * task)
    {
        this->mTaskState = ThreadState::Run;
        this->mWaitInvokeTask.Add(task);
        this->mThreadVariable.notify_one();
    }

    void TaskThread::Update()
    {
        TaskProxy *task = nullptr;
        this->mWaitInvokeTask.SwapQueueData();
        while (this->mWaitInvokeTask.PopItem(task))
        {
            task->Run();
            this->mFinishTasks.push(task->GetTaskId());
        }

        while (!this->mFinishTasks.empty())
        {
            unsigned int id = this->mFinishTasks.front();
            this->mFinishTasks.pop();
            this->mTaskComponent->PushFinishTask(id);
        }

        this->mTaskState = ThreadState::Idle;
        std::unique_lock<std::mutex> waitLock(this->mThreadLock);
        this->mThreadVariable.wait(waitLock);

    }
}

namespace Sentry
{
    NetWorkThread::NetWorkThread(TaskComponent *taskComponent, MethodProxy * method)
        : IThread(taskComponent), mMethodProxy(method)
    {

    }

    void NetWorkThread::AddTask(TaskProxy *task)
    {
        this->mWaitInvokeTask.Add(task);
    }

    void NetWorkThread::Update()
    {
        if(this->mMethodProxy)
        {
            this->mMethodProxy->run();
        }
        TaskProxy * taskProxy = nullptr;
        this->mWaitInvokeTask.SwapQueueData();
        while(this->mWaitInvokeTask.PopItem(taskProxy))
        {
            taskProxy->Run();
            this->mFinishTasks.emplace(taskProxy->GetTaskId());
        }

        while (!this->mFinishTasks.empty())
        {
            unsigned int id = this->mFinishTasks.front();
            this->mFinishTasks.pop();
            this->mTaskComponent->PushFinishTask(id);
        }
    }
}

// namespace Sentry
