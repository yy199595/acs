#include "ThreadTaskManager.h"

namespace SoEasy
{
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
					this->OnTaskFinish(taskAction);
				}
				this->mThreadTaskMap.erase(iter);
			}
		}
		while (!this->mWaitDispatchTasks.empty())
		{
			shared_ptr<ThreadTaskAction> taskAction = this->mWaitDispatchTasks.front();
			ThreadPool * threadPool = this->GetApp()->GetThreadPool();
			if (threadPool->StartTaskAction(taskAction))
			{
				this->mWaitDispatchTasks.pop();
				continue;
			}
			break;
		}
	}
}
