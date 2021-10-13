#include "TaskComponent.h"
#include <Util/NumberHelper.h>
#include <Core/App.h>
#include <Method/MethodProxy.h>
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

    NetWorkThread * TaskComponent::NewNetworkThread(const std::string & name,MethodProxy * method)
    {
        auto iter = this->mNetThread.find(name);
        if(iter == this->mNetThread.end())
        {
            NetWorkThread * thread = new NetWorkThread(this, method);
            this->mNetThread.emplace(name, thread);
            SayNoDebugLog("start new network thread [" <<  name << "]");
            return thread;
        }
        return iter->second;
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
        if (task == nullptr)
        {
            return false;
        }
        auto iter = this->mTaskMap.find(task->GetTaskId());
        if (iter == this->mTaskMap.end())
        {
            task->mTaskId = mTaskNumberPool.Pop();
            this->mTaskMap.emplace(task->GetTaskId(), task);
            size_t index = task->GetTaskId() % mThreadCount;
            this->mThreadArray[index]->AddTask(task);
            return true;
        }
        return false;
    }

    bool TaskComponent::StartTask(const std::string & name, TaskProxy * task)
    {
        if(task== nullptr)
        {
            return false;
        }
        auto iter1 = this->mNetThread.find(name);
        if(iter1 == this->mNetThread.end())
        {
            return false;
        }
        auto iter = this->mTaskMap.find(task->GetTaskId());
        if (iter == this->mTaskMap.end())
        {
            task->mTaskId = mTaskNumberPool.Pop();
            this->mTaskMap.emplace(task->GetTaskId(), task);
            iter1->second->AddTask(task);
            return true;
        }
        return false;
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
