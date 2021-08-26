#pragma once

#include<list>
#include<queue>
#include<memory>
#include<tuple>
#include<functional>
#include<unordered_map>
#include"Coroutine.h"

#include<Manager/Manager.h>
#include<NetWork/TcpClientSession.h>

#define CoroutinePoolMaxCount 100    //协程池最大数量
namespace Sentry
{
	class CoroutineManager : public Manager, public IFrameUpdate
	{
	public:
		CoroutineManager();

	public:
		template<typename F, typename T, typename ... Args>
		unsigned int Start(F && f, T * o, Args &&... args) {
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
		bool OnInit() final;

		void OnInitComplete() final;

		void OnFrameUpdate(float t) final;
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
		void SaveStack(Coroutine *, char *top);

	private:
		std::string mMessageBuffer;
		class TimerManager *mTimerManager;
	private:
		CoroutinePool mCorPool;
		Coroutine * mMainCoroutine;
		unsigned int mCurrentCorId;
		char mSharedStack[STACK_SIZE];
	};
}