//
// Created by yjz on 2022/1/5.
//
#include"TaskSourceBase.h"

namespace GameKeeper
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

    bool TaskSourceBase::ResumeTask()
    {
        if (this->mState == TaskState::TaskAwait)
        {
            if (this->mTaskScheduler.IsCurrentThread())
            {
                this->mTaskComponent->Resume(this->mCorId);
                return true;
            }
            this->mTaskScheduler.Invoke(&TaskComponent::Resume, this->mTaskComponent, this->mCorId);
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

