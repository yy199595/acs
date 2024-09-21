#pragma once
#include"Lua/Engine/Define.h"
#include"Rpc/Client/Message.h"
#include"Async/Coroutine/CoroutineDef.h"
#include"Async/Source/TaskSource.h"
#include"Async/Lua/LuaWaitTaskSource.h"

namespace acs
{
    template<typename T>
    class IRpcTask
    {
    public:
        explicit IRpcTask(int id) :mRpcId(id){ }
        virtual ~IRpcTask() = default;
    public:
        int GetRpcId() { return mRpcId; }
		virtual void OnResponse(T * response) = 0;
    private:
        int mRpcId;
    };

	class RpcTaskSource : public WaitTaskSourceBase, public IRpcTask<rpc::Packet>
    {
    public:
        explicit RpcTaskSource(int id): IRpcTask<rpc::Packet>(id), mMessage(nullptr) { }
    public:
		inline rpc::Packet * Await();
		inline void OnResponse(rpc::Packet * response) final;
	private:
		rpc::Packet * mMessage;
    };

	inline void RpcTaskSource::OnResponse(rpc::Packet* response)
	{
		this->mMessage = response;
		this->ResumeTask();
	}

	inline rpc::Packet * RpcTaskSource::Await()
	{
		this->YieldTask();
		return this->mMessage;
	}

	class LuaRpcTaskSource :  public IRpcTask<rpc::Packet>
	{
	 public:
		LuaRpcTaskSource(lua_State * lua, int id);
	 public:
		void OnResponse(rpc::Packet * response) final;
		inline int Await() { return this->mTask.Await(); }
	 private:
		LuaWaitTaskSource mTask;
	};
}