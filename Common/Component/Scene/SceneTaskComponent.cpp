#include "SceneTaskComponent.h"
#include <Util/NumberHelper.h>
#include <Core/App.h>
namespace Sentry
{
    SceneTaskComponent::SceneTaskComponent()
    {
        this->mThreadCount = 0;
    }

	bool SceneTaskComponent::Awake()
	{
		int count = 0;
		App::Get().GetConfig().GetValue("ThreadCount", count);
		this->mThreadCount = count != 0 ? count : (int)std::thread::hardware_concurrency();

		for (int index = 0; index < this->mThreadCount; index++)
		{
			TaskThread *taskThread = new TaskThread(this, index);
			mThreadArray.push_back(taskThread);
		}
		SayNoDebugLog("start new thread [ " << this->mThreadCount << " ]");
		return true;
	}

    void SceneTaskComponent::Start()
    {
		
    }

	void SceneTaskComponent::GetThreads(std::vector<std::thread::id> & threads)
	{
		for (TaskThread * taskThread : this->mThreadArray)
		{
			threads.push_back(taskThread->GetId());
		}
	}

	void SceneTaskComponent::PushFinishTask(unsigned int taskId)
	{
		this->mFinishTaskQueue.Add(taskId);
	}

	void SceneTaskComponent::PushFinishTask(std::queue<unsigned int> & tasks)
	{
		this->mFinishTaskQueue.AddRange(tasks);
	}

    long long SceneTaskComponent::CreateTaskId()
    {
        return NumberHelper::Create();
    }

    bool SceneTaskComponent::StartTask(TaskProxy * task)
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

    void SceneTaskComponent::OnSystemUpdate()
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
					delete task;
				}				             
                this->mTaskMap.erase(iter);
            }
			this->mTaskNumberPool.Push(taskId);
        }
    }
}
