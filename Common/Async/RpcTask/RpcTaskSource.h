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
        virtual int GetTimeout() = 0;
        virtual long long GetRpcId() = 0;
        virtual void OnResponse(std::shared_ptr<T> response) = 0;
    };

    class RpcTaskSource : public IRpcTask<com::Rpc::Response>
    {
    public:
        RpcTaskSource(float timeout = 0) : mTimeout(timeout * 1000) { }
        long long GetRpcId() final { return mTaskSource.GetTaskId(); }
    protected:
        int GetTimeout() final { return this->mTimeout;}
        void OnResponse(std::shared_ptr<com::Rpc_Response> response) final;

    public:
		std::shared_ptr<com::Rpc_Response> Await();
    private:
        const int mTimeout;
        TaskSource<std::shared_ptr<com::Rpc_Response>> mTaskSource;
    };

	class LuaRpcTaskSource :  public IRpcTask<com::Rpc::Response>
	{
	 public:
		LuaRpcTaskSource(lua_State * lua);
	 public:
		int Await() { return this->mTask.Await(); }
		long long GetRpcId() final { return this->mTaskId; }
	 private:
		int GetTimeout() final { return 0;}
		void OnResponse(std::shared_ptr<com::Rpc::Response> response) final;
	 private:
		long long mTaskId;
		LuaWaitTaskSource mTask;
	};
}