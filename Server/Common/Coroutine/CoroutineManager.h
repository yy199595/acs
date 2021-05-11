#pragma once
#include<list>
#include<queue>
#include<memory>
#include<tuple>
#include<functional>
#include<unordered_map>
#include"CoroutineEvent.h"
#ifndef _WIN32
#include<ucontext.h>
#endif
#include<Manager/Manager.h>
#include<NetWork/TcpClientSession.h>
namespace SoEasy
{
	struct Coroutine;
	class CoroutineManager : public Manager
	{
	public:
		CoroutineManager();
	public:
		long long Start(CoroutineAction func);
		long long Create(CoroutineAction func);
	public:
		void YieldReturn();
		void Sleep(long long ms);
		void Resume(long long id);
	protected:
		bool OnInit() override;
		void OnInitComplete() override;
		void OnFrameUpdate(float t) override;
	public:
		long long GetNowTime();
		void Destory(long long id);
		Coroutine * GetCoroutine();
		Coroutine * GetCoroutine(long long id);
		long long GetCurrentCorId() { return this->mCurrentCorId; }
	private:
#ifdef _WIN32
		void LoopCheckDelCoroutine();
#endif
	private:
		void SaveStack(Coroutine *, char * top);
	private:
		std::string mMessageBuffer;
		class TimerManager * mTimerManager;	
	private:
		long long mCurrentCorId;
#ifndef _WIN32
		ucontext_t mMainContext;
#else
		void * mMainCoroutineStack;
		std::queue<Coroutine *> mDestoryCoroutine;
#endif
		char mSharedStack[STACK_SIZE];
		std::unordered_map<long long, Coroutine *> mCoroutineMap;
	};
}