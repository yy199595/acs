#pragma once
#include<Script/LuaInclude.h>
#include<XCode/XCode.h>
#include<Util/NumberBuilder.h>
#include<Protocol/com.pb.h>
#include"Coroutine/CoroutineDef.h"
#include<tuple>
#include"Async/AsyncTask.h"
using namespace com;
using namespace google::protobuf;
namespace GameKeeper
{
    class TimerComponent;
    class TaskComponent;
    class ProtoResponseComponent;
    class ProtoRpcTask : public AsyncTask,
        public std::enable_shared_from_this<ProtoRpcTask>
    {
    public:
        explicit ProtoRpcTask(XCode code);
        explicit ProtoRpcTask(int method, long long rpcId);
    public:
        void OnTaskAwait() final;
        int GetMethodId() const { return this->mMethod;}
        long long GetRpcId() const { return this->mRpcId;}
        unsigned int GetTimerId() const { return this->mTimerId;}
        virtual void OnResponse(const com::Rpc_Response * response) = 0;
    private:
        int mMethod;
        long long mRpcId;
    protected:
        XCode mCode;
        unsigned int mTimerId;
    };

    class LuaProtoRpcTask : public ProtoRpcTask
    {
    public:
        LuaProtoRpcTask(int method, lua_State *lua, lua_State * cor)
            : ProtoRpcTask(method, 0), mCoroutine(cor), luaEnv(lua) {}
    public:
        void OnResponse(const com::Rpc_Response * backData) final;
    private:
		int ref;
		lua_State *luaEnv;
		lua_State * mCoroutine;
    };


    class CppProtoRpcTask : public ProtoRpcTask
    {
    public:
        explicit CppProtoRpcTask(XCode code);
        explicit CppProtoRpcTask(int methodId, long long rpcId);
        ~CppProtoRpcTask() final;
    public:
        void OnResponse(const com::Rpc_Response * backData) final;

        XCode Await();

        XCode Await(std::shared_ptr<Message> response);

    private:
        XCode mRespCode;
        std::shared_ptr<Message> mMessage;
        ProtoResponseComponent * mResponseComponent;
    };
}