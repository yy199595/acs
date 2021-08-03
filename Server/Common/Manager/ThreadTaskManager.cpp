#include "ThreadTaskManager.h"
#include <Thread/ThreadTaskAction.h>
#include <Util/NumberHelper.h>

namespace Sentry
{
    ThreadTaskManager::ThreadTaskManager()
    {
        this->mThreadCount = 0;
    }

    bool ThreadTaskManager::OnInit()
    {
        int count = this->GetThreadCount();
        for (int index = 0; index < count; index++)
        {
            TaskThread *taskThread = new TaskThread(this, index);
            if (taskThread == nullptr)
            {
                return false;
            }
            mThreadArray.push_back(taskThread);
        }
        SayNoDebugLog("start new thread [ " << this->mThreadCount << " ]");
        return true;
    }

    void ThreadTaskManager::OnInitComplete()
    {

    }

    int ThreadTaskManager::GetThreadCount()
    {
        if (this->mThreadCount == 0)
        {
            int count = 0;
            this->GetConfig().GetValue("ThreadCount", count);
            this->mThreadCount = count != 0 ? count : (int) std::thread::hardware_concurrency();
        }
        return this->mThreadCount;
    }

    long long ThreadTaskManager::CreateTaskId()
    {
        return NumberHelper::Create();
    }

    void ThreadTaskManager::OnTaskFinish(long long taskId)
    {
        mFinishTaskQueue.AddItem(taskId);
    }

    bool ThreadTaskManager::StartInvokeTask(std::shared_ptr<ThreadTaskAction> taskAction)
    {
        if (taskAction == nullptr || !taskAction->InitTaskAction(this))
        {
            return false;
        }
        if (this->mThreadIndex == this->mThreadArray.size())
        {
            this->mThreadIndex = 0;
        }
        this->mThreadTaskMap.emplace(taskAction->GetTaskId(), taskAction);
        this->mThreadArray[this->mThreadIndex]->AddTaskAction(taskAction);
        return true;
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
