#include "TaskComponent.h"
#include <Util/NumberHelper.h>
#include <Core/App.h>
namespace Sentry
{
    TaskComponent::TaskComponent()
    {
        this->mThreadCount = 0;
    }

	bool TaskComponent::Awake()
	{
		int count = 0;
		App::Get().GetConfig().GetValue("ThreadCount", count);
		this->mThreadCount = count != 0 ? count : (int)std::thread::hardware_concurrency();

		for (int index = 0; index < this->mThreadCount; index++)
		{
			TaskThread *taskThread = new TaskThread(this);
			mThreadArray.push_back(taskThread);
		}
		return true;
	}

    void TaskComponent::Start()
    {
		
    }

	void TaskComponent::PushFinishTask(unsigned int taskId)
	{
		this->mFinishTaskQueue.Add(taskId);
	}

	void TaskComponent::PushFinishTask(std::queue<unsigned int> & tasks)
	{
		this->mFinishTaskQueue.AddRange(tasks);
	}

    long long TaskComponent::CreateTaskId()
    {
        return NumberHelper::Create();
    }

    bool TaskComponent::StartTask(TaskProxy * task)
    {             
		if (task != nullptr)
		{
			auto iter = this->mTaskMap.find(task->GetTaskId());
			if (iter == this->mTaskMap.end())
			{
				task->mTaskId = mTaskNumberPool.Pop();
				this->mTaskMap.emplace(task->GetTaskId(), task);
				size_t index = task->GetTaskId() % mThreadCount;
				this->mThreadArray[index]->AddTask(task);
				return true;
			}
		}     
        return true;
    }

    void TaskComponent::OnSystemUpdate()
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
