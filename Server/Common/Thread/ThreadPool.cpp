#include"ThreadPool.h"

namespace SoEasy
{
	ThreadPool::ThreadPool(unsigned int count)
	{
		unsigned int num = count == 0 ? std::thread::hardware_concurrency() : count;
		for (unsigned int index = 0; index < num; index++)
		{
			TaskThread * taskThread = new TaskThread();
			if (taskThread != nullptr)
			{
				SayNoDebugInfo("start thread id : " << taskThread->GetThreadId());
				this->mThreadMap.emplace(taskThread->GetThreadId(), taskThread);
			}		
		}
	}

	TaskThread * ThreadPool::GetTaskThread(long long id)
	{
		auto iter = this->mThreadMap.find(id);
		return iter != this->mThreadMap.end() ? iter->second : nullptr;
	}

	void ThreadPool::GetAllTaskThread(std::vector<long long> & threads)
	{
		for (auto iter = this->mThreadMap.begin(); iter != this->mThreadMap.end(); iter++)
		{
			threads.push_back(iter->first);
		}
	}

	bool ThreadPool::StartTaskAction(std::shared_ptr<ThreadTaskAction> taskAction)
	{
		if (this->mThreadMap.empty())
		{
			return false;
		}
		auto iter = this->mThreadMap.begin();
		size_t size = iter->second->GetTaskSize();
		long long threadId = iter->second->GetThreadId();
		for (; iter != this->mThreadMap.end(); iter++)
		{
			TaskThread * taskThread = iter->second;
			if (taskThread->GetTaskSize() == 0)
			{
				taskThread->AddTaskAction(taskAction);
				return true;
			}
			if (taskThread->GetTaskSize() < size)
			{
				size = taskThread->GetTaskSize();
				threadId = taskThread->GetThreadId();
			}
		}
		TaskThread * taskThread = this->GetTaskThread(threadId);
		taskThread->AddTaskAction(taskAction);
		return true;
	}
}
