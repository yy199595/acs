#pragma once

#include<queue>
#include<memory>
#include<functional>
#include"Entity/Component/Component.h"
#include"Async/Coroutine/TaskContext.h"

namespace acs
{
	class CoroutineComponent final : public Component,
			public ISystemUpdate, public ILastFrameUpdate, public IServerRecord
	{
	 public:
		CoroutineComponent();
	 public:
		template<typename F, typename T, typename ... Args>
		inline unsigned int Start(F&& f, T* o, Args&& ... args)
		{
			TaskContext* co = this->MakeContext(
				NewMethodProxy(std::forward<F>(f), o, std::forward<Args>(args)...));
			this->Resume(co->mCoroutineId);
			return co->mCoroutineId;
		}
		inline unsigned int Start(std::function<void()>&& func)
		{
			TaskContext* co = this->MakeContext(
                    new LambdaMethod(std::move(func)));
			this->Resume(co->mCoroutineId);
			return co->mCoroutineId;
		}
	 public:
		void Sleep(unsigned int ms);
		void Resume(unsigned int id);
		bool YieldCoroutine() const;
		bool YieldCoroutine(unsigned int& mCorId) const;
    private:
		bool Awake() final;
		bool LateAwake() final;
		void OnSystemUpdate() final;
		void OnLastFrameUpdate() final;
		void OnRecord(json::w::Document &document) final;
	 public:
		void RunTask(tb_context_t context);
		inline unsigned int GetContextId() const
		{
			return this->mRunContext == nullptr ? 0 :
				   this->mRunContext->mCoroutineId;
		}
		size_t Count() const { return this->mCorPool.GetCount(); }
	 private:
		void SaveStack(unsigned int id);
		void RunCoroutine(TaskContext * coroutine);
		TaskContext* MakeContext(StaticMethod* func);
    private:
		TaskContextPool mCorPool;
		TaskContext* mRunContext;
		tb_context_t mMainContext;
		Stack mSharedStack[SHARED_STACK_NUM];
		std::queue<unsigned int> mLastQueues;
		class TimerComponent * mTimerComponent;
        std::queue<TaskContext  *> mResumeContexts;
	};
}