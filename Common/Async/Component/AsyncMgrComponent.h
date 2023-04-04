#pragma once

#include<list>
#include<queue>
#include<stack>
#include<memory>
#include<tuple>
#include<functional>
#include"Core/Component/Component.h"
#include"Async/Coroutine/TaskContext.h"

namespace Tendo
{
	class AsyncMgrComponent final : public Component, public ISystemUpdate, public ILastFrameUpdate
	{
	 public:
		AsyncMgrComponent();
	 public:
		template<typename F, typename T, typename ... Args>
		TaskContext* Start(F&& f, T* o, Args&& ... args)
		{
			TaskContext* co = this->MakeContext(
				NewMethodProxy(std::forward<F>(f), o, std::forward<Args>(args)...));
			this->Resume(co->mCoroutineId);
			return co;
		}
		TaskContext* Start(std::function<void()>&& func)
		{
			TaskContext* co = this->MakeContext(
                    new LambdaMethod(std::move(func)));
			this->Resume(co->mCoroutineId);
			return co;
		}

    private:
		TaskContext* MakeContext(StaticMethod* func);
	 public:
		bool YieldCoroutine();

		bool YieldCoroutine(unsigned int& mCorId);

		void Sleep(long long ms);

		void Resume(unsigned int id);

    private:
		bool Awake() final;

		void OnSystemUpdate() final;

		void OnLastFrameUpdate() final;
	 public:
		void RunTask(tb_context_t context);

		unsigned int GetContextId() const
		{
			return this->mRunContext == nullptr ? 0 :
				   this->mRunContext->mCoroutineId;
		}
	 private:
		void SaveStack(unsigned int id);
		void ResumeContext(TaskContext* co);
	 private:
		TaskContextPool mCorPool;
		TaskContext* mRunContext;
		tb_context_t mMainContext;
		Stack mSharedStack[SHARED_STACK_NUM];
		std::queue<unsigned int> mLastQueues;
        std::queue<unsigned int> mResumeContexts;
	};
}