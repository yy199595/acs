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
        explicit AsyncTask(XCode code = XCode::Successful);
        virtual ~AsyncTask() = default;

    public:
        long long GetCreateTime() const { return this->mCreateTime;}
    protected:
        bool AwaitTask();
        bool RestoreAsyncTask();
        virtual void OnTaskAwait() = 0;
    protected:
        XCode mCode;
        AsyncTaskState mTaskState;
    private:
        unsigned int mCorId;
        long long mCreateTime;
        class CoroutineComponent * mCorComponent;
    };
}


#endif //GAMEKEEPER_ASYNCTASK_H
