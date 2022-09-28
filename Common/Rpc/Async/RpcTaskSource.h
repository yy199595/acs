#pragma once
#include"Lua/LuaInclude.h"
#include"XCode/XCode.h"
#include"Client/Message.h"
#include"Guid/NumberBuilder.h"
#include"Coroutine/CoroutineDef.h"
#include"Source/TaskSource.h"
#include"Lua/LuaWaitTaskSource.h"
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
        virtual void OnTimeout() { throw std::logic_error("");};
        virtual void OnResponse(std::shared_ptr<T> response) = 0;
    private:
        int mTimeout;
    };

    class RpcTaskSource : public IRpcTask<Rpc::Data>
    {
    public:
        RpcTaskSource(int ms);
        long long GetRpcId() final { return mTaskSource.GetTaskId(); }
    protected:
        void OnTimeout() final;
        void OnResponse(std::shared_ptr<Rpc::Data> response) final;
    public:
		std::shared_ptr<Rpc::Data> Await();
    private:
#ifdef __DEBUG__
        long long t1;
#endif
        TaskSource<std::shared_ptr<Rpc::Data>> mTaskSource;
    };

	class LuaRpcTaskSource :  public IRpcTask<Rpc::Data>
	{
	 public:
		LuaRpcTaskSource(lua_State * lua, int ms, const std::string & response);
	 public:
		int Await() { return this->mTask.Await(); }
		long long GetRpcId() final { return this->mTaskId; }
	 private:
        void OnTimeout() final;
		void OnResponse(std::shared_ptr<Rpc::Data> response) final;
	 private:
		long long mTaskId;
		LuaWaitTaskSource mTask;
        const std::string mResp;
#ifdef __DEBUG__
       long long t1;
#endif
	};
}