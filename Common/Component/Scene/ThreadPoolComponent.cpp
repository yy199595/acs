#include "ThreadPoolComponent.h"
#include "Util/Guid.h"
#include "Object/App.h"
#include "Method/MethodProxy.h"
namespace Sentry
{
	bool ThreadPoolComponent::Awake()
    {
        this->mIndex = 0;
        int taskCount = 0;
        const ServerConfig & config = App::Get().GetConfig();
        config.GetMember("thread", "task", taskCount);
#ifndef ONLY_MAIN_THREAD
		int networkCount = 1;
        LOG_CHECK_RET_FALSE(config.GetValue("thread", "network", networkCount));
        for (int index = 0; index < networkCount; index++)
        {
            this->mNetThreads.push_back(new NetWorkThread());
        }
#endif
		for (int index = 0; index < taskCount; index++)
		{
			mThreadArray.push_back(new TaskThread());
		}
        return true;
    }

    bool ThreadPoolComponent::LateAwake()
    {
#ifndef ONLY_MAIN_THREAD
        for (auto taskThread: this->mNetThreads)
        {
            taskThread->Start();
        }
#endif
        for (auto taskThread: this->mThreadArray)
        {
            taskThread->Start();
        }
        return true;
    }

	void ThreadPoolComponent::OnDestory()
	{
		for(IThread * thread : this->mThreadArray)
		{
			thread->Stop();
			delete thread;
		}
		this->mThreadArray.clear();
	}

    void ThreadPoolComponent::GetAllThread(std::vector<const IThread *> &threads)
    {
        threads.clear();
        MainTaskScheduler & mainTask = App::Get().GetTaskScheduler();
#ifndef ONLY_MAIN_THREAD
        for(const IThread * taskThread : this->mNetThreads)
        {
            threads.emplace_back(taskThread);
        }
#endif
        for(const IThread * taskThread : this->mThreadArray)
        {
            threads.emplace_back(taskThread);
        }
        threads.emplace_back(&mainTask);
    }
#ifndef ONLY_MAIN_THREAD
    IAsioThread & ThreadPoolComponent::AllocateNetThread()
    {
        std::lock_guard<std::mutex> lock(this->mLock);
        if (this->mIndex >= mNetThreads.size())
        {
            this->mIndex = 0;
        }
        return *(mNetThreads[this->mIndex++]);
    }
#endif

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
