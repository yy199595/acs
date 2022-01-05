#pragma once

#include"RedisTaskBase.h"
#include"Async/TaskSource.h"

namespace GameKeeper
{
    class RedisInvokeResult
    {
    public:

    };
}

namespace GameKeeper
{
    class TaskComponent;

    class RedisTaskProxy : public RedisTaskBase
    {
    public:
        explicit RedisTaskProxy(const std::string &cmd);
    public:
        std::shared_ptr<RedisResponse> GetResponse() { return mTask.Await();}
    protected:
        void RunFinish() final;  //执行完成之后在主线程调用
        TaskSource<std::shared_ptr<RedisResponse>> mTask;
    };
}