#pragma once

#include<list>
#include<queue>
#include<memory>
#include<tuple>
#include<functional>
#include"Coroutine.h"
#include<Component/Component.h>
namespace Sentry
{
	class CoroutineComponent : public Component, public ISystemUpdate, public ILastFrameUpdate
	{
	public:
		CoroutineComponent();

	public:
		template<typename F, typename T, typename ... Args>
		unsigned int StartCoroutine(F && f, T * o, Args &&... args) {
			return this->StartCoroutine(
				NewMethodProxy(std::forward<F>(f), o, std::forward<Args>(args)...));
		}

	private:
		unsigned int StartCoroutine(MethodProxy * func);
	public:
		void YieldReturn();

		void Sleep(long long ms);

		void YieldNextLoop();

		void YieldNextFrame();

		void Resume(unsigned int id);

		CoroutineGroup * NewCoroutineGroup();

	protected:
		bool Awake() final;

		void Start() final;

		void OnSystemUpdate() final;

		void OnLastFrameUpdate() final;

		int GetPriority() override { return 1; }

	public:
		long long GetNowTime();

		void Destory(Coroutine * coroutine);

		Coroutine *GetCoroutine();

		Coroutine *GetCoroutine(unsigned int id);

		unsigned int GetCurrentCorId()
		{
			return this->mCurrentCorId;
		}

		bool IsInMainCoroutine()
		{
			return this->mCurrentCorId == 0;
		}

		bool IsInLogicCoroutine()
		{
			return this->mCurrentCorId != 0;
		}
	private:
		void Loop();
		void ResumeCoroutine();
#ifdef __COROUTINE_ASM__
		void SaveStack(Coroutine *);
#else
		void SaveStack(Coroutine *, char *top);
#endif
	private:
		std::string mMessageBuffer;
		class TimerComponent *mTimerManager;
		std::queue<unsigned int> mLateUpdateCors;
	private:
		CoroutinePool mCorPool;
		Coroutine * mMainCoroutine;
		unsigned int mCurrentCorId;
#ifdef __COROUTINE_ASM__
		Stack * mSharedStack;
#else
		char * mTop;
		char mSharedStack[STACK_SIZE];
#endif
		std::queue<unsigned int> mResumeCors;
		std::list<CoroutineGroup *> mCoroutineGroups;
	};
}