//
// Created by zmhy0073 on 2022/11/2.
//

#ifndef APP_HTTPTASK_H
#define APP_HTTPTASK_H
#include"Http/HttpResponse.h"
#include"Async/RpcTaskSource.h"

namespace Sentry
{
    class HttpRequestTask : public IRpcTask<Http::Response>
    {
    public:
        HttpRequestTask(int time = 0);
    public:
        long long GetRpcId() { return this->mTaskId;}

    public:
        void OnTimeout();
        void OnResponse(std::shared_ptr<Http::Response> response);
        std::shared_ptr<Http::Response> Await() { return this->mTask.Await();}
    private:
        long long mTaskId;
        TaskSource<std::shared_ptr<Http::Response>> mTask;
    };
    typedef std::shared_ptr<IRpcTask<Http::Response>> SharedHttpRpcTask;
}

namespace Sentry
{
    class HttpRequestClient;
    class LuaHttpRequestTask : public IRpcTask<Http::Response>
    {
    public:
        LuaHttpRequestTask(lua_State * lua);
        ~LuaHttpRequestTask();
    public:
        long long GetRpcId() final { return this->mTaskId; }
        int Await(std::shared_ptr<HttpRequestClient> client);
    public:
        void OnTimeout() final { }
        void OnResponse(std::shared_ptr<Http::Response> response) final;
    private:
        int mRef;
        lua_State * mLua;
        long long mTaskId;
        std::shared_ptr<HttpRequestClient> mRequestClient;
    };
}

#endif //APP_HTTPTASK_H
