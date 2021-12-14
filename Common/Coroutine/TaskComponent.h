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
    class TaskComponent : public Component, public ISystemUpdate, public ILastFrameUpdate, public ISecondUpdate
	{
	public:
		TaskComponent();
		~TaskComponent() final;
	public:
		template<typename F, typename T, typename ... Args>
		Coroutine * Start(F && f, T * o, Args &&... args) {
            Coroutine *co = this->CreateCoroutine(
                    NewMethodProxy(std::forward<F>(f),
                                   o, std::forward<Args>(args)...));
            this->Resume(co->mCoroutineId);
            return co;
        }
        Coroutine * Start(std::function<void()> func){
            Coroutine * co = this->CreateCoroutine(new LambdaMethod(func));
            this->Resume(co->mCoroutineId);
            return co;
        }
        Coroutine * CreateCoroutine(StaticMethod * func);
	public:
		void Await();

		void Await(unsigned int & mCorId);

		void AwaitSleep(long long ms);

		void Resume(unsigned int id);

		CoroutineGroup * NewCoroutineGroup();

	protected:
		bool Awake() final;

		bool LateAwake() final;

		void OnSystemUpdate() final;

		void OnLastFrameUpdate() final;

        void OnSecondUpdate() final;

		int GetPriority() override { return 2; }

	public:

		void Destory(Coroutine * coroutine);

		Coroutine *GetCoroutine(unsigned int id);

        Coroutine * GetCurCoroutine();

        unsigned int GetCurrentCorId() const
        {
            return this->mCurrentCorId;
        }

        void RunTask(tb_context_t context);

		bool IsInMainCoroutine() const
		{
			return this->mCurrentCorId == 0;
		}

		bool IsInLogicCoroutine() const
		{
			return this->mCurrentCorId != 0;
		}
	private:
        void Test(int index);
        void SaveStack(unsigned int id);
        void ResumeCoroutine(Coroutine * co);
	private:
		class TimerComponent *mTimerManager;
		std::queue<unsigned int> mLastQueues1;
		std::queue<unsigned int> mLastQueues2;
	private:
		CoroutinePool mCorPool;
        tb_context_t mMainContext;
        unsigned int mCurrentCorId;
		Stack mSharedStack[SHARED_STACK_NUM];
		std::queue<unsigned int> mResumeCoroutines;
        std::unordered_map<unsigned int, CoroutineGroup *> mCoroutineGroups;
	};
}