#pragma once

#include"RedisTaskBase.h"


namespace GameKeeper
{
    class RedisInvokeResult
    {
    public:

    };
}

namespace GameKeeper
{
    class CoroutineComponent;

    class RedisTaskProxy : public RedisTaskBase
    {
    public:
        explicit RedisTaskProxy(const std::string &cmd);

    public:
        long long GetCoroutineId() const { return mCoroutineId; }
        
    protected:
        void RunFinish() final;  //执行完成之后在主线程调用
    private:
        unsigned int mCoroutineId;
    };
}