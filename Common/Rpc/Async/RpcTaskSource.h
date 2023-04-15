#pragma once
#include"Lua/Engine/Define.h"
#include"Rpc/Client/Message.h"
#include"Util/Guid/NumberBuilder.h"
#include"Async/Coroutine/CoroutineDef.h"
#include"Async/Source/TaskSource.h"
#include"Async/Lua/LuaWaitTaskSource.h"
using namespace google::protobuf;
namespace Tendo
{
    template<typename T>
    class IRpcTask : public std::enable_shared_from_this<IRpcTask<T>>
    {
    public:
        explicit IRpcTask(int id) :mRpcId(id){ }
        virtual ~IRpcTask() = default;
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
        explicit RpcTaskSource(int ms);
 
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