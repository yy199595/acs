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
        IRpcTask(int id) :mRpcId(id){ }
        virtual ~IRpcTask() { }
    public:
        int GetRpcId() { return mRpcId; }
    public:
        virtual void OnResponse(std::shared_ptr<T> response) = 0;
    private:
        int mRpcId;
    };

    class RpcTaskSource : public IRpcTask<Rpc::Packet>
    {
    public:
        RpcTaskSource(int ms);
 
    protected:     
        void OnResponse(std::shared_ptr<Rpc::Packet> response) final;
    public:
		std::shared_ptr<Rpc::Packet> Await();
    private:
        TaskSource<std::shared_ptr<Rpc::Packet>> mTaskSource;
    };

	class LuaRpcTaskSource :  public IRpcTask<Rpc::Packet>
	{
	 public:
		LuaRpcTaskSource(lua_State * lua, int id, const std::string & response);
	 public:
		int Await() { return this->mTask.Await(); }	
	 private:
		void OnResponse(std::shared_ptr<Rpc::Packet> response) final;
	 private:	
		LuaWaitTaskSource mTask;
        const std::string mResp;
#ifdef __DEBUG__
       long long t1;
#endif
	};
}