//
// Created by zmhy0073 on 2022/11/2.
//

#ifndef APP_HTTPTASK_H
#define APP_HTTPTASK_H
#include"Http/Common/HttpResponse.h"
#include"Rpc/Async/RpcTaskSource.h"

namespace Sentry
{
    class HttpRequestTask : public IRpcTask<Http::IResponse>
    {
    public:
        explicit HttpRequestTask();
    public:
        void OnResponse(std::shared_ptr<Http::IResponse> response) final;
        std::shared_ptr<Http::IResponse> Await() { return this->mTask.Await();}
    private:        
        TaskSource<std::shared_ptr<Http::IResponse>> mTask;
    };
    typedef std::shared_ptr<IRpcTask<Http::DataResponse>> SharedHttpRpcTask;
}

namespace Sentry
{
    class HttpRequestClient;
    class LuaHttpRequestTask : public IRpcTask<Http::IResponse>
    {
    public:
        explicit LuaHttpRequestTask(lua_State * lua);
        ~LuaHttpRequestTask() final;
    public:     
        int Await();
    public:    
        void OnResponse(std::shared_ptr<Http::IResponse> response) final;
    private:
        int mRef;
        lua_State * mLua;
    };
}

#endif //APP_HTTPTASK_H
