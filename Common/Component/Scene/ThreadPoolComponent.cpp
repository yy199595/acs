#include "ThreadPoolComponent.h"
#include <Util/Guid.h>
#include "Object/App.h"
#include <Method/MethodProxy.h>
namespace Sentry
{
	bool ThreadPoolComponent::Awake()
    {
        this->mIndex = 0;
        int taskCount = 1;
        int networkCount = 1;
        App::Get().GetConfig().GetValue("Thread", "task", taskCount);
        App::Get().GetConfig().GetValue("Thread", "network", networkCount);

        for (int index = 0; index < taskCount; index++)
        {
            mThreadArray.push_back(new TaskThread());
        }

        for (int index = 0; index < networkCount; index++)
        {
            this->mNetThreads.push_back(new NetWorkThread());
        }
        return true;
    }

    bool ThreadPoolComponent::LateAwake()
    {
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

    void ThreadPoolComponent::GetAllThread(std::vector<const IThread *> &threads)
    {
        threads.clear();
        MainTaskScheduler & mainTask = App::Get().GetTaskScheduler();
        for(const IThread * taskThread : this->mNetThreads)
        {
            threads.emplace_back(taskThread);
        }
        for(const IThread * taskThread : this->mThreadArray)
        {
            threads.emplace_back(taskThread);
        }
        threads.emplace_back(&mainTask);
    }

	NetWorkThread & ThreadPoolComponent::AllocateNetThread()
    {
        std::lock_guard<std::mutex> lock(this->mLock);
        if (this->mIndex >= mNetThreads.size())
        {
            this->mIndex = 0;
        }
        return *(mNetThreads[this->mIndex++]);
    }

	bool ThreadPoolComponent::StartTask(TaskProxy * task)
	{
		if (task == nullptr)
		{
			return false;
		}
		task->mTaskId = mTaskNumberPool.Pop();
		size_t index = task->GetTaskId() % this->mThreadArray.size();
		this->mThreadArray[index]->AddTask(task);
		return true;
	}
}
