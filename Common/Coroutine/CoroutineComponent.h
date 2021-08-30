#pragma once

#include<list>
#include<queue>
#include<memory>
#include<tuple>
#include<functional>
#include"Coroutine.h"
#include<Component/Component.h>
#define CoroutinePoolMaxCount 100    //协程池最大数量
namespace Sentry
{
	class CoroutineComponent : public Component, public ISystemUpdate
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

		void Resume(unsigned int id);

	protected:
		bool Awake() final;

		void Start() final;

		void OnSystemUpdate() final;

		int GetPriority() override { return 1; }
	private:
		void Loop();
		void Loop2();

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
		void ResumeCoroutine();
		void SaveStack(Coroutine *, char *top);

	private:
		std::string mMessageBuffer;
		class TimerComponent *mTimerManager;
	private:
		CoroutinePool mCorPool;
		Coroutine * mMainCoroutine;
		unsigned int mCurrentCorId;
		char mSharedStack[STACK_SIZE];
		std::queue<unsigned int> mResumeCors;
	};
}