//
// Created by zmhy0073 on 2021/11/29.
//

#ifndef GAMEKEEPER_TASKSOURCE_H
#define GAMEKEEPER_TASKSOURCE_H
#include<XCode/XCode.h>
#include"Util/Guid.h"
#include"Util/TimeHelper.h"
#include"Core/App.h"
namespace GameKeeper
{
    enum class TaskState
    {
        TaskReady,
        TaskAwait,
        TaskFinish,
    };

    template<typename T>
    class TaskSource
    {
    public:
        explicit TaskSource()
            : mTaskScheduler(App::Get().GetTaskScheduler())
        {
            this->mCorId = 0;
            this->mState = TaskState::TaskReady;
            this->mTaskId = Helper::Guid::Create();
            this->mCreateTime = Helper::Time::GetMilTimestamp();
            this->mTaskComponent = App::Get().GetTaskComponent();
        }
        virtual ~TaskSource() = default;
    public:
        long long GetTaskId() const { return this->mTaskId;}
        TaskState GetState() const { return this->mTaskState; }
        long long GetCreateTime() const { return this->mCreateTime;}
        bool IsComplete() { return this->mTaskState == TaskState::TaskFinish;}

    public:
        const T & Await();
        virtual bool SetResult(T && result);
    private:
        T mData;
        TaskState mState;
        long long mTaskId;
        unsigned int mCorId;
        long long mCreateTime;
        TaskComponent * mTaskComponent;
        MainTaskScheduler & mTaskScheduler;
    };

    template<typename T>
    const T & TaskSource<T>::Await()
    {
        if(this->mState == TaskState::TaskReady)
        {
            this->mState = TaskState::TaskAwait;
            this->mTaskComponent->Yield(this->mCorId);
        }
        return this->mData;
    }

    template<typename T>
    bool TaskSource<T>::SetResult(T && result)
    {
        if (this->mState == TaskState::TaskAwait)
        {
            this->mData = std::move(result);
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
}

#endif //GAMEKEEPER_TASKSOURCE_H
