#include "TaskThread.h"
#include <Scene/ThreadPoolComponent.h>
#include <Method/MethodProxy.h>
#include"Util/TimeHelper.h"
using namespace std::chrono;

namespace Sentry
{
	IThread::IThread(std::string  name)
		: mName(std::move(name)), mIsClose(false)
    {		
		this->mIsWork = true;
    }

    void IThread::HangUp()
    {
        if(this->mIsWork)
        {
            this->mIsWork = false;
            std::unique_lock<std::mutex> waitLock(this->mThreadLock);
            this->mThreadVariable.wait(waitLock);
        }
    }

    void IThread::Stop()
    {
        this->mIsClose = true;
        this->mThreadVariable.notify_one();
    }

}

namespace Sentry
{
    TaskThread::TaskThread()
        : IThread("task")
    {
        this->mTaskState = Idle;
		this->mThread = nullptr;
        this->mThread = new std::thread(std::bind(&TaskThread::Update, this));
        this->mThread->detach();
    }

	int TaskThread::Start()
	{
        this->mIsWork = true;
        this->mThreadVariable.notify_one();
		return 0;
	}

	void TaskThread::AddTask(TaskProxy * task)
    {
        if(!this->mIsWork)
        {
            this->mIsWork = true;
            this->mThreadVariable.notify_one();
        }
        this->mWaitInvokeTask.Push(task);
        this->mTaskState = ThreadState::Run;
    }

    void TaskThread::Update()
    {
        this->mThreadId = std::this_thread::get_id();

        this->HangUp();
        TaskProxy *task = nullptr;
        while(!this->mIsClose)
        {
#ifdef __THREAD_LOCK__
            this->mWaitInvokeTask.SwapQueueData();
#endif
            this->mWaitInvokeTask.Swap();
            while(this->mWaitInvokeTask.Pop(task))
            {
                task->Run();
                this->mLastOperTime = Helper::Time::GetSecTimeStamp();
            }
            this->HangUp();
        }
    }
}

#ifndef ONLY_MAIN_THREAD
namespace Sentry
{
    NetWorkThread::NetWorkThread()
        : IAsioThread("net"), mAsioContext(nullptr)
    {
		this->mAsioContext = new AsioContext(1);
		this->mAsioWork = new AsioWork(*this->mAsioContext);
        this->mThread = new std::thread(std::bind(&NetWorkThread::Update, this));
        this->mThread->detach();
    }

	int NetWorkThread::Start()
	{
        this->mIsWork = true;
        this->mThreadVariable.notify_one();
		return 0;
	}

	void NetWorkThread::InvokeMethod(StaticMethod *task)
	{
		this->mWaitInvokeMethod.Push(task);
	}

    void NetWorkThread::Update()
    {
        this->mThreadId = std::this_thread::get_id();

        this->HangUp();
        asio::error_code err;
        StaticMethod * taskMethod = nullptr;
        std::chrono::milliseconds time(1);
        while(!this->mIsClose)
        {
            std::this_thread::sleep_for(time);
            this->mAsioContext->poll(err);                  
#ifdef __THREAD_LOCK__
            this->mWaitInvokeMethod.SwapQueueData();
#endif
            this->mWaitInvokeMethod.Swap();
            while (this->mWaitInvokeMethod.Pop(taskMethod))
            {
                taskMethod->run();
                delete taskMethod;
                this->mLastOperTime = Helper::Time::GetSecTimeStamp();
            }
            this->mLastOperTime = Helper::Time::GetSecTimeStamp();
        }
    }
}
#endif
namespace Sentry
{
	MainTaskScheduler::MainTaskScheduler(StaticMethod * method)
		: IAsioThread("main"), mMainMethod(method)
	{
		this->mThreadId = std::this_thread::get_id();
        this->mAsioContext = new AsioContext(1);
        this->mAsioWork = new AsioWork(*this->mAsioContext);
	}

	int MainTaskScheduler::Start()
	{
        this->mIsWork = true;
		std::chrono::milliseconds time(1);
		while (!this->mIsClose)
		{
			this->Update();
			this_thread::sleep_for(time);			
		}
		return 0;
	}

	void MainTaskScheduler::Update()
	{
        mAsioContext->poll();
		this->mMainMethod->run();
		StaticMethod * task = nullptr;
#ifdef __THREAD_LOCK__
		this->mTaskQueue.SwapQueueData();
#endif
        this->mTaskQueue.Swap();
		while (this->mTaskQueue.Pop(task))
		{
			task->run();
			delete task;
		}
        this->mLastOperTime = Helper::Time::GetSecTimeStamp();
    }
	
	void MainTaskScheduler::InvokeMethod(StaticMethod * task)
	{
        if(task == nullptr) return;
		this->mTaskQueue.Push(task);
	}
}

// namespace Sentry
