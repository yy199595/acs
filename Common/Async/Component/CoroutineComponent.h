#pragma once

#include<list>
#include<queue>
#include<memory>
#include<functional>
#include"Entity/Component/Component.h"
#include"Async/Coroutine/TaskContext.h"

namespace Tendo
{
	class CoroutineComponent final : public Component, public ISystemUpdate, public ILastFrameUpdate
	{
	 public:
		CoroutineComponent();
	 public:
		template<typename F, typename T, typename ... Args>
		unsigned int Start(F&& f, T* o, Args&& ... args)
		{
			TaskContext* co = this->MakeContext(
				NewMethodProxy(std::forward<F>(f), o, std::forward<Args>(args)...));
			this->Resume(co->mCoroutineId);
			return co->mCoroutineId;
		}
		unsigned int Start(std::function<void()>&& func)
		{
			TaskContext* co = this->MakeContext(
                    new LambdaMethod(std::move(func)));
			this->Resume(co->mCoroutineId);
			return co->mCoroutineId;
		}
	 public:
		bool YieldCoroutine() const;

		bool YieldCoroutine(unsigned int& mCorId) const;

		void Sleep(unsigned int ms);

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
		void WaitAll(std::vector<unsigned int> & coroutines);
	 private:
#ifdef 	__COR_SHARED_STACK__
		void SaveStack(unsigned int id);
#endif
		void ResumeContext(TaskContext* co);
        TaskContext* MakeContext(StaticMethod* func);
    private:
		TaskContextPool mCorPool;
		TaskContext* mRunContext;
		tb_context_t mMainContext;
#ifdef __COR_SHARED_STACK__
		Stack mSharedStack[SHARED_STACK_NUM];
#endif
		std::queue<unsigned int> mLastQueues;
        std::queue<unsigned int> mResumeContexts;
	};
}