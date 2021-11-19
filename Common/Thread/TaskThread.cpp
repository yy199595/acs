#include "TaskThread.h"
#include <Scene/TaskPoolComponent.h>
#include <Method/MethodProxy.h>
#include"Util/TimeHelper.h"
using namespace std::chrono;

namespace GameKeeper
{
	IThread::IThread(std::string  name,TaskPoolComponent * task)
		: mName(std::move(name)), mIsClose(false), mTaskComponent(task)
    {		
		this->mIsWork = false;
    }

    void IThread::HangUp()
    {
        this->mIsWork = false;
        std::unique_lock<std::mutex> waitLock(this->mThreadLock);
        this->mThreadVariable.wait(waitLock);
    }

    void IThread::Stop()
    {
        this->mIsClose = true;
        this->mThreadVariable.notify_one();
    }

}

namespace GameKeeper
{
    TaskThread::TaskThread(TaskPoolComponent * taskComponent)
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
        this->mWaitInvokeTask.Add(task);
        this->mThreadVariable.notify_one();
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
            while (this->mWaitInvokeTask.PopItem(task))
            {
                this->mIsWork = true;
                if (task->Run())
                {
                    this->mFinishTasks.push(task->GetTaskId());
                }
            }

            while (!this->mFinishTasks.empty())
            {
                unsigned int id = this->mFinishTasks.front();
                this->mFinishTasks.pop();
                this->mTaskComponent->PushFinishTask(id);
            }
            this->HangUp();
            this->mTaskState = ThreadState::Idle;
        }

    }
}

namespace GameKeeper
{
    NetWorkThread::NetWorkThread(TaskPoolComponent *taskComponent)
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
            this->mLastOperTime = TimeHelper::GetSecTimeStamp();
            this->mAsioContext->poll(err);
            if(err)
            {
                GKDebugError(err.message());
            }           
            StaticMethod * taskMethod = nullptr;
#ifdef __THREAD_LOCK__
            this->mWaitInvokeMethod.SwapQueueData();
#endif
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
	
	void MainTaskScheduler::AddMainTask(StaticMethod * task)
	{
		if (task == nullptr)
		{
			return;
		}
		this->mTaskQueue.Add(task);
	}
}

// namespace GameKeeper
