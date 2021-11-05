#include "TaskPoolComponent.h"
#include <Util/NumberHelper.h>
#include <Core/App.h>
#include <Method/MethodProxy.h>
namespace GameKeeper
{
	bool TaskPoolComponent::Awake()
    {
        this->mIndex = 0;
        int taskCount = 1;
        int networkCount = 1;
        App::Get().GetConfig().GetValue("Thread", "task", taskCount);
        App::Get().GetConfig().GetValue("Thread", "network", networkCount);

        for (int index = 0; index < taskCount; index++)
        {
            mThreadArray.push_back(new TaskThread(this));
        }

        for (int index = 0; index < networkCount; index++)
        {
            this->mNetThreads.push_back(new NetWorkThread(this));
        }

        for (auto taskThread: this->mNetThreads)
        {
            taskThread->Start();
        }

        for (auto taskThread: this->mThreadArray)
        {
            taskThread->Start();
        }


        return true;
    }

    void TaskPoolComponent::Start()
    {
//		for (auto thread : this->mNetThreads)
//		{
//			thread->Start();
//		}
//
//		for (auto thread : mThreadArray)
//		{
//			thread->Start();
//		}
    }

	NetWorkThread & TaskPoolComponent::GetNetThread()
    {
        std::lock_guard<std::mutex> lock(this->mLock);
        if (this->mIndex >= mNetThreads.size())
        {
            this->mIndex = 0;
        }
        return *(mNetThreads[this->mIndex++]);
    }
	
    void TaskPoolComponent::PushFinishTask(unsigned int taskId)
	{
		this->mFinishTaskQueue.Add(taskId);
	}

    bool TaskPoolComponent::StartTask(TaskProxy * task)
    {
        if (task == nullptr)
        {
            return false;
        }
        auto iter = this->mTaskMap.find(task->GetTaskId());
        if (iter == this->mTaskMap.end())
        {
            task->mTaskId = mTaskNumberPool.Pop();
            this->mTaskMap.emplace(task->GetTaskId(), task);
            size_t index = task->GetTaskId() % this->mThreadArray.size();
            this->mThreadArray[index]->AddTask(task);
            return true;
        }
        return false;
    }

    void TaskPoolComponent::OnSystemUpdate()
    {
        unsigned int taskId = 0;
        this->mFinishTaskQueue.SwapQueueData();
        while (this->mFinishTaskQueue.PopItem(taskId))
        {
            auto iter = this->mTaskMap.find(taskId);
            if (iter != this->mTaskMap.end())
            {
                TaskProxy * task = iter->second;
				if (task != nullptr)
				{
					task->RunFinish();
				}				             
                this->mTaskMap.erase(iter);
            }
			this->mTaskNumberPool.Push(taskId);
        }
    }
}
