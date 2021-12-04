//
// Created by zmhy0073 on 2021/11/29.
//

#include"AsyncTask.h"
#include"Core/App.h"
#include"Util/TimeHelper.h"
namespace GameKeeper
{
    AsyncTask::AsyncTask(XCode code)
    {
        this->mCorId = 0;
        this->mCode = code;
        this->mCorComponent = nullptr;
        this->mCreateTime = TimeHelper::GetMilTimestamp();
        this->mTaskState = code == XCode::Successful ? TaskReady : TaskError;
    }

    bool AsyncTask::AsyncAwaitTask()
    {
        if(this->mTaskState == TaskReady)
        {
            this->OnTaskAwait();
            this->mTaskState = TaskAwait;
            this->mCorComponent = App::Get().GetCorComponent();
            this->mCorComponent->WaitForYield(this->mCorId);
            return true;
        }
        return false;
    }

    bool AsyncTask::RestoreAsyncTask()
    {
        if(this->mCorComponent == nullptr || this->mCorId == 0)
        {
            return false;
        }
        this->mCorComponent->Resume(this->mCorId);
        this->mCorId = 0;
        return true;
    }
}