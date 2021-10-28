#include "TaskThread.h"
#include <Scene/TaskPoolComponent.h>
#include <functional>
#include <Method/MethodProxy.h>
using namespace std::chrono;

namespace Sentry
{
	IThread::IThread(TaskPoolComponent * task)
		: mIsClose(false), mTaskComponent(task)
    {		
		
    }
}

namespace Sentry
{
    TaskThread::TaskThread(TaskPoolComponent * taskComponent)
        : IThread(taskComponent)
    {
        this->mTaskState = Idle;
		this->mThread = nullptr;
    }

	int TaskThread::Start()
	{
		this->mThread = new std::thread([this]()
		{
			std::chrono::milliseconds time(1);
			while (!this->mIsClose)
			{
				this->Update();
				//std::this_thread::sleep_for(time);
			}
		});
		this->mThread->detach();
		this->mThreadId = this->mThread->get_id();
		return 0;
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
            if(task->Run())
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

        this->mTaskState = ThreadState::Idle;
        std::unique_lock<std::mutex> waitLock(this->mThreadLock);
        this->mThreadVariable.wait(waitLock);

    }
}

namespace Sentry
{
    NetWorkThread::NetWorkThread(TaskPoolComponent *taskComponent, StaticMethod * method)
        : IThread(taskComponent), mMethodProxy(method), mAsioContext(nullptr)
    {
		this->mAsioContext = new AsioContext(1);
		this->mAsioWork = new AsioWork(*this->mAsioContext);
    }

	int NetWorkThread::Start()
	{
		this->mThread = new std::thread([this]()
		{
			std::chrono::milliseconds time(1);
			while (!this->mIsClose)
			{
				this->Update();
				//std::this_thread::sleep_for(time);
			}
		});
		this->mThread->detach();
		this->mThreadId = this->mThread->get_id();
		return 0;
	}

	void NetWorkThread::AddTask(TaskProxy *task)
    {
        this->mWaitInvokeTask.Add(task);
    }

	void NetWorkThread::AddTask(StaticMethod * task)
	{
		this->mWaitInvokeMethod.Add(task);
	}

	void NetWorkThread::Update()
    {
        asio::error_code err;
		this->mAsioContext->poll(err);
        if(err)
        {
            SayNoDebugError(err.message());
        }
        if(this->mMethodProxy)
        {
            this->mMethodProxy->run();
        }
        TaskProxy * taskProxy = nullptr;
		StaticMethod * taskMethod = nullptr;
        this->mWaitInvokeTask.SwapQueueData();
		this->mWaitInvokeMethod.SwapQueueData();
        while(this->mWaitInvokeTask.PopItem(taskProxy))
        {
            if(taskProxy->Run())
            {
				unsigned int id = taskProxy->GetTaskId();
				this->mTaskComponent->PushFinishTask(id);
            }
        }

		while (this->mWaitInvokeMethod.PopItem(taskMethod))
		{
			taskMethod->run();
            delete taskMethod;
		}     
    }
}

namespace Sentry
{
	MainTaskScheduler::MainTaskScheduler(StaticMethod * method)
		: IThread(nullptr), mMainMethod(method)
	{
		
	}

	int MainTaskScheduler::Start()
	{		
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
		this->mMainMethod->run();
		StaticMethod * task = nullptr;
		this->mTaskQueue.SwapQueueData();
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

// namespace Sentry
