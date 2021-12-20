#pragma once
#include<Script/LuaInclude.h>
#include<XCode/XCode.h>
#include<Util/NumberBuilder.h>
#include<Protocol/com.pb.h>
#include"Coroutine/CoroutineDef.h"
#include<tuple>
#include"Async/Task.h"
#ifdef __DEBUG__
#include"Other/ElapsedTimer.h"
#endif
using namespace com;
using namespace google::protobuf;
namespace GameKeeper
{
    class TaskComponent;
    class TimerComponent;
    class ProtoResponseComponent;
    class RpcTaskBase : public std::enable_shared_from_this<RpcTaskBase>
    {
    public:
        explicit RpcTaskBase(XCode code);
        explicit RpcTaskBase(int method);
        virtual ~RpcTaskBase() = default;
    public:
        int GetMethodId() const { return this->mMethod;}
        long long GetTaskId() const { return this->mTaskId;}
        double GetCostTime() const { return this->mTimer.GetMs();}
        bool SetResult(const Rpc_Response * result);
    public:
        XCode GetCode();
    protected:
        void AwaitTask();
        virtual void OnResponse(const Rpc_Response * response) = 0;
    protected:
        XCode mCode;
    private:
        int mMethod;
        TaskState mState;
        long long mTaskId;
        unsigned int mCorId;
#ifdef __DEBUG__
        ElapsedTimer mTimer;
#endif
        TaskComponent * mTaskComponent;
        static class RpcComponent * mRpcComponent;
    };

    class LuaRpcTask : public RpcTaskBase
    {
    public:
        LuaRpcTask(int method, lua_State *lua, lua_State * cor)
            : RpcTaskBase(method), mCoroutine(cor), luaEnv(lua) {}
    public:
       void OnResponse(const Rpc_Response *response) final;
    private:
		int ref;
		lua_State *luaEnv;
		lua_State * mCoroutine;
    };


    class RpcTask : public RpcTaskBase
    {
    public:
        explicit RpcTask(XCode code);
        explicit RpcTask(int methodId);
        ~RpcTask() final = default;
    public:
        template<typename T>
        std::shared_ptr<T> GetData();
    protected:
        void OnResponse(const com::Rpc_Response * backData) final;
    private:
        std::shared_ptr<Message> mMessage;
        ProtoResponseComponent * mResponseComponent;
    };
    template<typename T>
    std::shared_ptr<T> RpcTask::GetData()
    {
        this->AwaitTask();
        return dynamic_pointer_cast<T>(mMessage);
    }
}