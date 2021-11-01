#include "TaskPoolComponent.h"
#include <Util/NumberHelper.h>
#include <Core/App.h>
#include <Method/MethodProxy.h>
namespace GameKeeper
{
    TaskPoolComponent::TaskPoolComponent()
    {
        this->mThreadCount = 0;
    }

	bool TaskPoolComponent::Awake()
	{
		int count = 0;
		App::Get().GetConfig().GetValue("ThreadCount", count);
		this->mThreadCount = count != 0 ? count : (int)std::thread::hardware_concurrency();

		for (int index = 0; index < this->mThreadCount; index++)
		{
			mThreadArray.push_back(new TaskThread(this));
		}
		for (TaskThread * taskThread : this->mThreadArray)
		{
			taskThread->Start();
		}
		return true;
	}

    void TaskPoolComponent::Start()
    {
		
    }

    NetWorkThread * TaskPoolComponent::NewNetworkThread(const std::string & name,StaticMethod * method)
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

    void TaskPoolComponent::PushFinishTask(unsigned int taskId)
	{
		this->mFinishTaskQueue.Add(taskId);
	}

	void TaskPoolComponent::PushFinishTask(std::queue<unsigned int> & tasks)
	{
		this->mFinishTaskQueue.AddRange(tasks);
	}

    long long TaskPoolComponent::CreateTaskId()
    {
        return NumberHelper::Create();
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
            size_t index = task->GetTaskId() % mThreadCount;
            this->mThreadArray[index]->AddTask(task);
            return true;
        }
        return false;
    }

    bool TaskPoolComponent::StartTask(const std::string & name, TaskProxy * task)
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
