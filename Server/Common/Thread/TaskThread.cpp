#include "TaskThread.h"
#include <chrono>
#include <sstream>
#include <functional>
#include<Manager/ThreadTaskManager.h>
using namespace std::chrono;
namespace SoEasy
{
	TaskThread::TaskThread(ThreadTaskManager * manager)
	{
		this->mThreadId = 0;
		this->mTaskState = Idle;
		this->mTaskManager = manager;
		std::stringstream streamBuffer;
		this->mBindThread = new std::thread(std::bind(&TaskThread::Run, this));
		if (this->mBindThread != nullptr)
		{
			streamBuffer << this->mBindThread->get_id();
			streamBuffer >> this->mThreadId;
		}
	}

	void TaskThread::WaitToNextWake()
	{
		std::unique_lock<std::mutex> lck(this->mThreadLock);
		this->mThreadVarible.wait(lck);
	}

	void TaskThread::AddTaskAction(SharedThreadTask taskAction)
	{
		this->mTaskState = ThreadState::Running;
		this->mWaitInvokeTask.AddItem(taskAction);
		this->mThreadVarible.notify_one();
	}

	void TaskThread::Run()
	{
		while (true)
		{
			SharedThreadTask taskAction;
			this->mWaitInvokeTask.SwapQueueData();

			while (this->mWaitInvokeTask.PopItem(taskAction))
			{
				taskAction->InvokeInThreadPool(this->mThreadId);
				taskAction->NoticeToMainThread(); // 通知到主线程
			}

			this->mTaskState = ThreadState::Idle;
			std::unique_lock<std::mutex> waitLock(this->mThreadLock);
			this->mThreadVarible.wait(waitLock);
		}
	}
}
