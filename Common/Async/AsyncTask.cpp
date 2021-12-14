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
        this->mTaskComponent = nullptr;
        this->mCreateTime = TimeHelper::GetMilTimestamp();
        this->mTaskState = code == XCode::Successful ? TaskReady : TaskError;
    }

    bool AsyncTask::AwaitTask()
    {
        if(this->mTaskState == TaskReady)
        {
            this->OnTaskAwait();
            this->mTaskState = TaskAwait;
            this->mTaskComponent = App::Get().GetTaskComponent();
            this->mTaskComponent->Await(this->mCorId);
            return true;
        }
        return false;
    }

    bool AsyncTask::RestoreAsyncTask()
    {
        if(this->mTaskComponent == nullptr || this->mCorId == 0)
        {
            return false;
        }
        this->mTaskComponent->Resume(this->mCorId);
        this->mCorId = 0;
        return true;
    }
}