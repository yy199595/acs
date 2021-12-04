#pragma once

#include<list>
#include<queue>
#include<stack>
#include<memory>
#include<tuple>
#include<functional>
#include"Coroutine.h"
#include<Component/Component.h>
namespace GameKeeper
{
    class CoroutineComponent : public Component, public ISystemUpdate, public ILastFrameUpdate, public ISecondUpdate
	{
	public:
		CoroutineComponent();
		~CoroutineComponent() final;
	public:
		template<typename F, typename T, typename ... Args>
		Coroutine * StartCoroutine(F && f, T * o, Args &&... args)
        {
            Coroutine *co = this->CreateCoroutine(
                    NewMethodProxy(std::forward<F>(f), o, std::forward<Args>(args)...));
            if(co != nullptr)
            {
                this->Resume(co->mCoroutineId);
                return co;
            }
            return nullptr;
        }
        Coroutine * CreateCoroutine(StaticMethod * func);
	public:
		void WaitForYield();

		void WaitForYield(unsigned int & mCorId);

		void WaitForSleep(long long ms);

		void Resume(unsigned int id);

		CoroutineGroup * NewCoroutineGroup();

	protected:
		bool Awake() final;

		//void Start() final;

		void OnSystemUpdate() final;

		void OnLastFrameUpdate() final;

        void OnSecondUpdate() final;

		int GetPriority() override { return 2; }

	public:

		void Destory(Coroutine * coroutine);

		Coroutine *GetCoroutine(unsigned int id);

        Coroutine * GetCurCoroutine();

		Coroutine * GetMainCoroutine() { return this->mMainCoroutine; }

        unsigned int GetCurrentCorId() const
        {
            return this->mCurrentCorId;
        }

		bool IsInMainCoroutine() const
		{
			return this->mCurrentCorId == 0;
		}

		bool IsInLogicCoroutine() const
		{
			return this->mCurrentCorId != 0;
		}
	private:
        void SleepTest();
		void ResumeCoroutine(Coroutine * co);
#ifdef __COROUTINE_ASM__
		void SaveStack(unsigned int id);
#else
		void SaveStack(Coroutine *, char *top);
#endif
	private:
		class TimerComponent *mTimerManager;
		std::queue<unsigned int> mLastQueues1;
		std::queue<unsigned int> mLastQueues2;
	private:
		CoroutinePool mCorPool;
		Coroutine * mMainCoroutine;
        unsigned int mCurrentCorId{};
#ifdef __COROUTINE_ASM__
		Stack mSharedStack[SHARED_STACK_NUM];
#else
		char * mTop;
		char mSharedStack[STACK_SIZE];
#endif
		std::queue<unsigned int> mResumeCoroutines;
        std::unordered_map<unsigned int, CoroutineGroup *> mCoroutineGroups;
	};
}