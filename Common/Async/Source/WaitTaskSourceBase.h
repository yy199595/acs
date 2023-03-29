//
// Created by zmhy0073 on 2021/11/29.
//

#ifndef GAMEKEEPER_TASKSOURCEBASE_H
#define GAMEKEEPER_TASKSOURCEBASE_H
#include"App/App.h"
#include"Guid/Guid.h"
#include"XCode/XCode.h"
#include"Time/TimeHelper.h"
namespace Sentry
{
    enum class TaskState
    {
        TaskReady,
        TaskAwait,
        TaskFinish,
    };

    class WaitTaskSourceBase
    {
    public:
        explicit WaitTaskSourceBase();
        virtual ~WaitTaskSourceBase() = default;
    public:
		void Clear();
        TaskState GetState() const { return this->mState; }
        long long GetTaskId() const { return this->mTaskId;}
        long long GetCreateTime() const { return this->mCreateTime;}
        bool IsComplete() { return this->mState == TaskState::TaskFinish;}
    protected:
        bool YieldTask();
        bool ResumeTask(TaskState state = TaskState::TaskFinish);
    private:
        TaskState mState;
        long long mTaskId;
        unsigned int mCorId;
        long long mCreateTime;
    };
}

#endif //GAMEKEEPER_TASKSOURCEBASE_H