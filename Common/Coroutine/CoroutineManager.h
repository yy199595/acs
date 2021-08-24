#pragma once

#include<list>
#include<queue>
#include<memory>
#include<tuple>
#include<functional>
#include<unordered_map>
#include"Coroutine.h"

#ifndef _WIN32
#include<ucontext.h>
#endif

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

		template<typename F, typename T>
		unsigned int Start(F && f, T * o) {
			return this->StartCoroutine(new_closure(std::forward<F>(f), o));
		}

		template<typename F, typename T,typename P>
		unsigned int Start(F && f, T * o,P && p) {
			return this->StartCoroutine(new_closure(std::forward<F>(f), o, std::forward<P>(p)));
		}

		template<typename F,typename T, typename P1, typename P2>
		unsigned int Start(F && f, T * t,P1 && p1, P2 && p2) {
			return this->StartCoroutine(new_closure(std::forward<F>(f), t, std::forward<P>(p1), std::forward<P>(p2)));
		}
	private:
		unsigned int StartCoroutine(Closure * func);
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