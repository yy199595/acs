//
// Created by zmhy0073 on 2022/11/2.
//

#ifndef APP_HTTPTASK_H
#define APP_HTTPTASK_H
#include"Http/Common/HttpResponse.h"
#include"Rpc/Async/RpcTaskSource.h"

namespace Sentry
{
    class HttpRequestTask : public IRpcTask<Http::Response>
    {
    public:
        explicit HttpRequestTask(int id);
    public:
        void OnTimeout();
        void OnResponse(std::shared_ptr<Http::Response> response) final;
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
        explicit LuaHttpRequestTask(lua_State * lua);
        ~LuaHttpRequestTask() final;
    public:     
        int Await();
    public:    
        void OnResponse(std::shared_ptr<Http::Response> response) final;
    private:
        int mRef;
        lua_State * mLua;
    };
}

#endif //APP_HTTPTASK_H
