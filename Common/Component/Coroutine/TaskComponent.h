#pragma once

#include<list>
#include<queue>
#include<stack>
#include<memory>
#include<tuple>
#include<functional>
#include"Coroutine/TaskContext.h"
#include"Component/Component.h"
#include"Other/DoubleQueue.h"

namespace Sentry
{
	class TaskComponent : public Component,
						  public ISystemUpdate, public ILastFrameUpdate, public ISecondUpdate
	{
	 public:
		TaskComponent() = default;
		~TaskComponent() final = default;
	 public:
		template<typename F, typename T, typename ... Args>
		TaskContext* Start(F&& f, T* o, Args&& ... args)
		{
			TaskContext* co = this->MakeContext(
				NewMethodProxy(std::forward<F>(f),
					o, std::forward<Args>(args)...));
			this->Resume(co->mCoroutineId);
			return co;
		}
		TaskContext* Start(std::function<void()>&& func)
		{
			TaskContext* co = this->MakeContext(new LambdaMethod(std::move(func)));
			this->Resume(co->mCoroutineId);
			return co;
		}
		TaskContext* MakeContext(StaticMethod* func);
	 public:
		bool YieldCoroutine();

		bool YieldCoroutine(unsigned int& mCorId);

		void Sleep(long long ms);

		void Resume(unsigned int id);

		void WhenAny(TaskContext* coroutine);

		void WhenAll(std::vector<TaskContext*>& coroutines);

	 protected:
		bool Awake() final;

		bool LateAwake() final;

		void OnSystemUpdate() final;

		void OnLastFrameUpdate() final;

		void OnSecondUpdate() final;

	 public:

		TaskContext* GetContext(unsigned int id);

		unsigned int GetContextId() const
		{
			return this->mRunContext == nullptr ? 0 :
				   this->mRunContext->mCoroutineId;
		}

		void RunTask(tb_context_t context);

	 private:
		void Test(int index);
		void SaveStack(unsigned int id);
		void ResumeContext(TaskContext* co);
	 private:
		class TimerComponent* mTimerManager;
		std::queue<unsigned int> mLastQueues;
	 private:
		TaskContextPool mCorPool;
		tb_context_t mMainContext;
		TaskContext* mRunContext;
		Stack mSharedStack[SHARED_STACK_NUM];
		DoubleQueue<unsigned int> mResumeContexts;
	};
}