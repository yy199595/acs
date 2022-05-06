#pragma once
#include<Script/LuaInclude.h>
#include<XCode/XCode.h>
#include<Util/NumberBuilder.h>
#include<Protocol/com.pb.h>
#include"Coroutine/CoroutineDef.h"
#include<tuple>
#include"Async/TaskSource.h"
#ifdef __DEBUG__
#include"Other/ElapsedTimer.h"
#endif
using namespace com;
using namespace google::protobuf;
namespace Sentry
{
    class IRpcTask : public std::enable_shared_from_this<IRpcTask>
    {
    public:
        virtual int GetTimeout() = 0;
        virtual long long GetRpcId() = 0;
        virtual void OnResponse(std::shared_ptr<com::Rpc_Response> response) = 0;
    };

    class LuaRpcTaskSource : public IRpcTask
    {
    public:
        LuaRpcTaskSource(lua_State *lua, lua_State *coroutine);
    public:
        int GetTimeout() final { return 0;}
        long long GetRpcId() final { return this->mRpcId;}
        void OnResponse(std::shared_ptr<com::Rpc_Response> response) final;
    private:
        int ref;
        long long mRpcId;
        lua_State * mluaEnv;
        lua_State *mCoroutine;
    };

    class RpcHandlerComponent;
    class RpcTaskSource : public IRpcTask
    {
    public:
        RpcTaskSource(float timeout = 0) : mTimeout(timeout * 1000) { }
        long long GetRpcId() final { return mTaskSource.GetTaskId(); }
    protected:
        int GetTimeout() final { return this->mTimeout;}
        void OnResponse(std::shared_ptr<com::Rpc_Response> response) final;

    public:
        XCode AwaitCode();
        template<typename T>
        std::shared_ptr<T> AwaitData();
		std::shared_ptr<com::Rpc_Response> Await();
    private:
        const int mTimeout;
        RpcHandlerComponent * mRpcComponent;
        TaskSource<std::shared_ptr<com::Rpc_Response>> mTaskSource;
    };
    template<typename T>
    std::shared_ptr<T> RpcTaskSource::AwaitData()
    {
        auto response = mTaskSource.Await();
        if(response == nullptr || !response->has_data() || !response->data().Is<T>())
        {
            return nullptr;
        }
        std::shared_ptr<T> data = std::make_shared<T>();
        return response->data().UnpackTo(data.get()) ? data : nullptr;
    }
}