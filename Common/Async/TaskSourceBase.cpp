//
// Created by yjz on 2022/1/5.
//
#include"TaskSourceBase.h"

namespace Sentry
{
    TaskSourceBase::TaskSourceBase()
            : mTaskScheduler(App::Get().GetTaskScheduler())
    {
        this->mCorId = 0;
        this->mState = TaskState::TaskReady;
        this->mTaskId = Helper::Guid::Create();
        this->mCreateTime = Helper::Time::GetMilTimestamp();
        this->mTaskComponent = App::Get().GetTaskComponent();
    }

    bool TaskSourceBase::ResumeTask(TaskState state)
    {
        if (this->mState == TaskState::TaskAwait)
        {
            this->mState = state;
            this->mTaskComponent->Resume(this->mCorId);
            return true;
        }
        return false;
    }

    bool TaskSourceBase::YieldTask()
    {
        if(this->mState == TaskState::TaskReady)
        {
            this->mState = TaskState::TaskAwait;
            this->mTaskComponent->Yield(this->mCorId);
            return true;
        }
        return false;
    }
}

