#include"TaskThread.h"
#include<chrono>
#include<sstream>
using namespace std::chrono;
namespace SoEasy
{
	TaskThread::TaskThread()
	{
		this->mThreadId = 0;
		this->mTaskState = Idle;
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

	bool TaskThread::AddTaskAction(SharedThreadTask taskAction)
	{
		if (this->mTaskState == Idle)
		{
			this->mThreadLock.lock();
			this->mWaitInvokeTask.push(taskAction);
			this->mThreadLock.unlock();
			this->mThreadVarible.notify_one();
			return true;
		}
		return false;
	}

	void TaskThread::Run()
	{
		while (true)
		{
			if (this->mWaitInvokeTask.empty())
			{
				this->mTaskState = Idle;
				std::unique_lock<std::mutex> waitLock(this->mThreadLock);
				this->mThreadVarible.wait(waitLock);
			}
			else
			{
				this->mThreadLock.lock();
				this->mTaskState = Running;
				SharedThreadTask taskAction = this->mWaitInvokeTask.front();
				this->mWaitInvokeTask.pop();
				this->mThreadLock.unlock();

				taskAction->InvokeInThreadPool(this->mThreadId);
				taskAction->NoticeToMainThread();// 通知到主线程
			}
		}
	}
}
