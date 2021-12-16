//
// Created by zmhy0073 on 2021/11/29.
//

#include"AsyncTask.h"
#include"Core/App.h"
#include"Util/TimeHelper.h"
namespace GameKeeper
{
    AsyncTask::AsyncTask()
    {
        this->mCorId = 0;
        this->mTaskState = TaskReady;
        this->mCreateTime = TimeHelper::GetMilTimestamp();
        this->mTaskComponent = App::Get().GetTaskComponent();
    }

    bool AsyncTask::AwaitTask()
    {
        if(this->mTaskState == TaskReady)
        {
            this->OnTaskAwait();
            this->mTaskState = TaskAwait;
            this->mTaskComponent->Await(this->mCorId);
            return true;
        }
        return false;
    }

    bool AsyncTask::RestoreTask(AsyncTaskState state)
    {
        if(this->mTaskState == TaskAwait)
        {
            this->mTaskState = state;
            this->mTaskComponent->Resume(this->mCorId);
            return true;
        }
        return false;
    }
}