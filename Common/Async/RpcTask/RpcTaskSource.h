#pragma once
#include<Script/LuaInclude.h>
#include<XCode/XCode.h>
#include<Util/NumberBuilder.h>
#include<Message/com.pb.h>
#include"Coroutine/CoroutineDef.h"
#include"Async/TaskSource.h"
#include"Async/Lua/LuaWaitTaskSource.h"
using namespace com;
using namespace google::protobuf;
namespace Sentry
{
    template<typename T>
    class IRpcTask : public std::enable_shared_from_this<IRpcTask<T>>
    {
    public:
        IRpcTask(int ms) : mTimeout(ms) { }
        virtual ~IRpcTask() { }
    public:
        virtual long long GetRpcId() = 0;
        int GetTimeout() const { return this->mTimeout; };
    public:
        virtual void OnTimeout() { throw logic_error("");};
        virtual void OnResponse(std::shared_ptr<T> response) = 0;
    private:
        int mTimeout;
    };

    class RpcTaskSource : public IRpcTask<com::Rpc::Response>
    {
    public:
        RpcTaskSource(int ms) : IRpcTask<com::Rpc::Response>(ms) { }
        long long GetRpcId() final { return mTaskSource.GetTaskId(); }
    protected:
        void OnTimeout() final;
        void OnResponse(std::shared_ptr<com::Rpc_Response> response) final;
    public:
		std::shared_ptr<com::Rpc_Response> Await();
    private:
        TaskSource<std::shared_ptr<com::Rpc_Response>> mTaskSource;
    };

	class LuaRpcTaskSource :  public IRpcTask<com::Rpc::Response>
	{
	 public:
		LuaRpcTaskSource(lua_State * lua, int ms);
	 public:
		int Await() { return this->mTask.Await(); }
		long long GetRpcId() final { return this->mTaskId; }
	 private:
        void OnTimeout() final;
		void OnResponse(std::shared_ptr<com::Rpc::Response> response) final;
	 private:
		long long mTaskId;
		LuaWaitTaskSource mTask;
	};
}