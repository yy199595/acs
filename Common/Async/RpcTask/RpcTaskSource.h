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
namespace GameKeeper
{
    class IRpcTask : public std::enable_shared_from_this<IRpcTask>
    {
    public:
        virtual int GetTimeout() = 0;
        virtual long long GetRpcId() = 0;
        virtual void OnResponse(const com::Rpc_Response * response) = 0;
    };

    class LuaRpcTaskSource : public IRpcTask
    {
    public:
        LuaRpcTaskSource(lua_State *lua, lua_State *coroutine);
    public:
        int GetTimeout() final { return 0;}
        long long GetRpcId() final { return this->mRpcId;}
        void OnResponse(const Rpc_Response *response) final;
    private:
        int ref;
        long long mRpcId;
        lua_State * mluaEnv;
        lua_State *mCoroutine;
    };

    class RpcComponent;
    class RpcTaskSource : public IRpcTask
    {
    public:
        RpcTaskSource(int timeout = 0) : mTimeout(timeout) { }
        long long GetRpcId() final { return mTaskSource.GetTaskId(); }
    protected:
        int GetTimeout() final { return this->mTimeout;}
        void OnResponse(const com::Rpc_Response *response) final;
    public:
        XCode GetCode();
        template<typename T>
        std::shared_ptr<T> GetData();
    private:
        int mTimeout;
        RpcComponent * mRpcComponent;
        TaskSource<com::Rpc_Response *> mTaskSource;
    };
    template<typename T>
    std::shared_ptr<T> RpcTaskSource::GetData()
    {
        const com::Rpc_Response * response = mTaskSource.Await();
        if(response == nullptr || !response->has_data())
        {
            return nullptr;
        }
        const Any & any = response->data();
        if(!response->data().Is<T>())
        {
            return nullptr;
        }
        std::shared_ptr<T> data(new T());
        if(!response->data().UnpackTo(data.get()))
        {
            return nullptr;
        }
        return data;
    }
}