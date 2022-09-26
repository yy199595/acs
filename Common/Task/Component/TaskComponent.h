#pragma once

#include<list>
#include<queue>
#include<stack>
#include<memory>
#include<tuple>
#include<functional>
#include"Component/Component.h"
#include"Coroutine/TaskContext.h"

namespace Sentry
{
	class TaskComponent final : public Component,
                                public ISystemUpdate, public ILastFrameUpdate
#ifdef __DEBUG__
            , public ISecondUpdate
#endif
	{
	 public:
		TaskComponent() = default;
		~TaskComponent() final = default;
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
		void Awake() final;

		void OnSystemUpdate() final;

		void OnLastFrameUpdate() final;
#ifdef __DEBUG__
        void OnSecondUpdate(const int tick) final;
#endif
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
		tb_context_t mMainContext;
		TaskContext* mRunContext;
		Stack mSharedStack[SHARED_STACK_NUM];
		std::queue<unsigned int> mLastQueues;
		std::queue<unsigned int> mResumeContexts;
	};
}