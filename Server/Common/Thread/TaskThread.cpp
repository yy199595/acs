#include"TaskThread.h"
#include<chrono>
using namespace std::chrono;
namespace SoEasy
{
	TaskThread::TaskThread()
	{
		this->mThreadId = 0;
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
		this->mThreadLock.lock();
		this->mTaskBuffer.push(taskAction);
		this->mThreadLock.unlock();
		this->mThreadVarible.notify_one();
	}

	void TaskThread::Run()
	{
		while (true)
		{		
			if (this->mTaskBuffer.empty())
			{
				std::unique_lock<std::mutex> waitLock(this->mThreadLock);
				this->mThreadVarible.wait(waitLock);
			}
			this->mThreadLock.lock();
			std::swap(this->mTaskBuffer, this->mWaitInvokeTask);	
			this->mThreadLock.unlock();

			while (!this->mWaitInvokeTask.empty())
			{
				SharedThreadTask taskAction = this->mWaitInvokeTask.front();
				this->mWaitInvokeTask.pop();
				taskAction->InvokeInThreadPool(this->mThreadId);	
				taskAction->NoticeToMainThread();// 通知到主线程
			}
		}
	}
}
