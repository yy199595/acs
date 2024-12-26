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
		inline unsigned int Start(F&& f, T* o, Args&& ... args) noexcept
		{
			std::unique_ptr<StaticMethod> method = NewMethodProxy(std::forward<F>(f), o, std::forward<Args>(args)...);
			TaskContext* co = this->MakeContext(std::move(method));
			if(co == nullptr)
			{
				return 0;
			}
			return this->Resume(co->mCoroutineId);
		}
		inline unsigned int Start(std::function<void()>&& func) noexcept
		{
			std::unique_ptr<LambdaMethod> method = std::make_unique<LambdaMethod>(std::move(func));
			{
				TaskContext* co = this->MakeContext(std::move(method));
				if(co == nullptr)
				{
					return 0;
				}
				return this->Resume(co->mCoroutineId);
			}
		}
	 public:
		void Sleep(unsigned int ms);
		bool YieldCoroutine()  const noexcept;
		unsigned int Resume(unsigned int id) noexcept;
		bool YieldCoroutine(unsigned int& mCorId) const noexcept;
    private:
		bool Awake() final;
		bool LateAwake() final;
		void OnSystemUpdate() noexcept final;
		void OnLastFrameUpdate(long long) noexcept final;
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
		TaskContext* MakeContext(std::unique_ptr<StaticMethod> func);
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