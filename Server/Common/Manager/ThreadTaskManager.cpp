#include "ThreadTaskManager.h"
#include <Thread/ThreadTaskAction.h>
#include <Util/NumberHelper.h>
namespace SoEasy
{

	bool ThreadTaskManager::OnInit()
	{
		this->mThreadCount = (int)std::thread::hardware_concurrency();
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("ThreadCount", this->mThreadCount));	
		return true;
	}

	void ThreadTaskManager::OnInitComplete()
	{
		for (int index = 0; index < this->mThreadCount; index++)
		{
			TaskThread *taskThread = new TaskThread(this);
			if (taskThread != nullptr)
			{
				mThreadArray.push_back(taskThread);
				SayNoDebugLog("start new thread id " << taskThread->GetThreadId());
			}
		}
	}

	long long ThreadTaskManager::CreateTaskId()
	{
		return NumberHelper::Create();
	}

	void ThreadTaskManager::OnTaskFinish(long long taskId)
	{
		mFinishTaskQueue.AddItem(taskId);
	}

	void ThreadTaskManager::GetAllTaskThread(std::vector<long long>& threads)
	{
		threads.clear();
		for (TaskThread* task : this->mThreadArray)
		{
			threads.push_back(task->GetThreadId());
		}
	}

	long long ThreadTaskManager::StartInvokeTask(std::shared_ptr<ThreadTaskAction> taskAction)
	{
		if (taskAction == nullptr || !taskAction->InitTaskAction(this))
		{
			return 0;
		}		
		if(this->mThreadIndex == this->mThreadArray.size())
		{
			this->mThreadIndex = 0;
		}
		this->mThreadTaskMap.emplace(taskAction->GetTaskId(), taskAction);
		this->mThreadArray[this->mThreadIndex]->AddTaskAction(taskAction);
		return taskAction->GetTaskId();
	}

	void ThreadTaskManager::OnSystemUpdate()
	{
		long long taskId = 0;
		this->mFinishTaskQueue.SwapQueueData();
		while (this->mFinishTaskQueue.PopItem(taskId))
		{
			auto iter = this->mThreadTaskMap.find(taskId);
			if (iter != this->mThreadTaskMap.end())
			{
				SharedThreadTask taskAction = iter->second;
				if (taskAction != nullptr)
				{
					taskAction->OnTaskFinish();
				}
				this->mThreadTaskMap.erase(iter);
			}
		}
	}
}
