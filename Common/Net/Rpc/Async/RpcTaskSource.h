#pragma once
#include"Lua/Engine/Define.h"
#include"Rpc/Client/Message.h"
#include"Async/Coroutine/CoroutineDef.h"
#include"Async/Source/TaskSource.h"
#include"Async/Lua/LuaWaitTaskSource.h"
#ifdef __SHARE_PTR_COUNTER__
#include "Core/Memory/MemoryObject.h"
#endif
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
		virtual void OnResponse(std::unique_ptr<T> response) = 0;
    private:
        int mRpcId;
    };

	class RpcTaskSource : public WaitTaskSourceBase, public IRpcTask<rpc::Message>
#ifdef __SHARE_PTR_COUNTER__
	, public memory::Object<LuaWaitTaskSource>
#endif
    {
    public:

        explicit RpcTaskSource(int id): IRpcTask<rpc::Message>(id), mMessage(nullptr) { }
    public:
		inline std::unique_ptr<rpc::Message> Await();
		inline void OnResponse(std::unique_ptr<rpc::Message> response) final;
	private:
		std::unique_ptr<rpc::Message> mMessage;
    };

	inline void RpcTaskSource::OnResponse(std::unique_ptr<rpc::Message> response)
	{
		this->mMessage = std::move(response);
		this->ResumeTask();
	}

	inline std::unique_ptr<rpc::Message> RpcTaskSource::Await()
	{
		this->YieldTask();
		return std::move(this->mMessage);
	}

	class LuaRpcTaskSource :  public IRpcTask<rpc::Message>
#ifdef __SHARE_PTR_COUNTER__
	, public memory::Object<LuaWaitTaskSource>
#endif
	{
	 public:
		LuaRpcTaskSource(lua_State * lua, int id);
	 public:
		void OnResponse(std::unique_ptr<rpc::Message> response) final;
		inline int Await() { return this->mTask.Await(); }
	 private:
		LuaWaitTaskSource mTask;
	};
}