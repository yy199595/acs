#include "ThreadPool.h"

namespace SoEasy
{
	unsigned int ThreadPool::mThreadCount = 0;
	std::vector<TaskThread *> ThreadPool::mTaskThreadList;
	void ThreadPool::InitPool(unsigned int count)
	{
		mThreadCount = std::thread::hardware_concurrency();
		mThreadCount = count == 0 ? mThreadCount : mThreadCount == 0 ? 2 : mThreadCount;
		for (unsigned int index = 0; index < mThreadCount; index++)
		{
			TaskThread * taskThread = new TaskThread();
			if (taskThread != nullptr)
			{
				SayNoDebugInfo("start thread id : " << taskThread->GetThreadId());
				mTaskThreadList.push_back(taskThread);
			}		
		}
	}

	TaskThread * ThreadPool::GetTaskThread(long long id)
	{
		for (size_t index = 0; index < mTaskThreadList.size(); index++)
		{
			TaskThread * taskThread = mTaskThreadList[index];
			if (taskThread->GetThreadId() == id)
			{
				return taskThread;
			}
		}
		return nullptr;
	}

	void ThreadPool::GetAllTaskThread(std::vector<TaskThread *> & threads)
	{
		for (size_t index = 0; index < mTaskThreadList.size(); index++)
		{
			TaskThread * taskThread = mTaskThreadList[index];
			threads.push_back(taskThread);
		}
	}

	bool ThreadPool::StartTaskAction(std::shared_ptr<ThreadTaskAction> taskAction)
	{
		std::sort(mTaskThreadList.begin(), mTaskThreadList.end(), [](TaskThread * t1, TaskThread * t2)
		{
			return t1->GetTaskSize() < t2->GetTaskSize();
		});
		auto iter = mTaskThreadList.begin();
		if (iter != mTaskThreadList.end())
		{
			TaskThread * taskThread = (*iter);
			taskThread->AddTaskAction(taskAction);
			return true;
		}
		return false;
	}
}
