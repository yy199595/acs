#include "TaskThread.h"
#include <Scene/ThreadPoolComponent.h>
#include <Method/MethodProxy.h>
#include"Util/TimeHelper.h"
using namespace std::chrono;

namespace GameKeeper
{
	IThread::IThread(std::string  name,ThreadPoolComponent * task)
		: mName(std::move(name)), mIsClose(false), mTaskComponent(task)
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

namespace GameKeeper
{
    TaskThread::TaskThread(ThreadPoolComponent * taskComponent)
        : IThread("task", taskComponent)
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
        this->mWaitInvokeTask.Add(task);
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
            this->mLastOperTime = TimeHelper::GetSecTimeStamp();
            if (this->mWaitInvokeTask.PopItem(task))
            {
                if (task->Run())
                {
                    unsigned int id = task->GetTaskId();
                    this->mTaskComponent->PushFinishTask(id);
                }
                continue;
            }
            this->HangUp();
        }
    }
}

namespace GameKeeper
{
    NetWorkThread::NetWorkThread(ThreadPoolComponent *taskComponent)
        : IThread("net", taskComponent), mAsioContext(nullptr)
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

	void NetWorkThread::AddTask(StaticMethod * task)
	{
		this->mWaitInvokeMethod.Add(task);
	}

    void NetWorkThread::Update()
    {
        this->mThreadId = std::this_thread::get_id();

        this->HangUp();
        asio::error_code err;
        std::chrono::milliseconds time(1);
        while(!this->mIsClose)
        {
            std::this_thread::sleep_for(time);
            this->mAsioContext->poll(err);                  
            StaticMethod * taskMethod = nullptr;
#ifdef __THREAD_LOCK__
            this->mWaitInvokeMethod.SwapQueueData();
#endif
            this->mLastOperTime = TimeHelper::GetSecTimeStamp();
            while (this->mWaitInvokeMethod.PopItem(taskMethod))
            {
                taskMethod->run();
                delete taskMethod;
            }
        }
    }
}

namespace GameKeeper
{
	MainTaskScheduler::MainTaskScheduler(StaticMethod * method)
		: IThread("main", nullptr), mMainMethod(method)
	{
		this->mThreadId = std::this_thread::get_id();
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
        this->mLastOperTime = TimeHelper::GetSecTimeStamp();
		this->mMainMethod->run();
		StaticMethod * task = nullptr;
#ifdef __THREAD_LOCK__
		this->mTaskQueue.SwapQueueData();
#endif
		while (this->mTaskQueue.PopItem(task))
		{
			task->run();
			delete task;
		}
	}
	
	void MainTaskScheduler::Invoke(StaticMethod * task)
	{
		if (task == nullptr)
		{
			return;
		}
		this->mTaskQueue.Add(task);
	}
}

// namespace GameKeeper
