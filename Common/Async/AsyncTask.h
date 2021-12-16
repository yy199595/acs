//
// Created by zmhy0073 on 2021/11/29.
//

#ifndef GAMEKEEPER_ASYNCTASK_H
#define GAMEKEEPER_ASYNCTASK_H
#include<XCode/XCode.h>
namespace GameKeeper
{
    enum AsyncTaskState
    {
        TaskReady,
        TaskError,
        TaskAwait,
        TaskTimeout,
        TaskFinish,
    };

    class AsyncTask
    {
    public:
        explicit AsyncTask();
        virtual ~AsyncTask() = default;

    public:
        AsyncTaskState GetState() const { return this->mTaskState; }
        long long GetCreateTime() const { return this->mCreateTime;}
        bool IsComplete() { return this->mTaskState == AsyncTaskState::TaskFinish;}
    protected:
        bool AwaitTask();
        virtual void OnTaskAwait() = 0;
        bool RestoreTask(AsyncTaskState state);
    private:
        unsigned int mCorId;
        long long mCreateTime;
        AsyncTaskState mTaskState;
        class TaskComponent * mTaskComponent;
    };
}


#endif //GAMEKEEPER_ASYNCTASK_H
