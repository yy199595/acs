#pragma once
#include<Script/LuaInclude.h>
#include<XCode/XCode.h>
#include<Util/NumberBuilder.h>
#include<Protocol/com.pb.h>
#include"Coroutine/CoroutineDef.h"
#include<tuple>
#include"Async/TaskSource.h"
#ifdef __DEBUG__
#include"Script/LuaParameter.h"
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
		std::shared_ptr<com::Rpc_Response> Await();
    private:
        const int mTimeout;
        TaskSource<std::shared_ptr<com::Rpc_Response>> mTaskSource;
    };

	class LuaRpcTaskSource
	{
	public:
		LuaRpcTaskSource(lua_State * lua);
		~LuaRpcTaskSource();
	public:
		int Yield();
		void SetResult();
		template<typename T>
		void SetResult(T result);
		void SetJson(const std::string & json);
		void SetResult(XCode code, std::shared_ptr<Message> response);
	private:
		int mRef;
		lua_State * mLua;
	};

	template<typename T>
	void LuaRpcTaskSource::SetResult(T result)
	{
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		lua_State* coroutine = lua_tothread(this->mLua, -1);
		Lua::Parameter::Write(this->mLua, result);
		lua_presume(coroutine, this->mLua, 1);
	}
}