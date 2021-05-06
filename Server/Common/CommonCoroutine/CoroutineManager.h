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
#include<CommonManager/Manager.h>
#include<CommonNetWork/TcpClientSession.h>
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
	private:
		void WakeUpCoroutine(long long id);
	public:
		XCode Call(shared_ptr<TcpClientSession> session, const std::string func, Message & returnData);
		XCode Call(shared_ptr<TcpClientSession> session, const std::string func, const Message * message = nullptr);
		XCode Call(shared_ptr<TcpClientSession> session, const std::string func, const Message * message, Message & returnData);
	private:
		XCode SendCallMessage(shared_ptr<TcpClientSession> session, const std::string & func, const Message * message, class NetWorkRetActionBox * callBack);
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
		void LoopCoroutineEvent();
	private:
		void SaveStack(Coroutine *, char * top);
	private:
		std::string mMessageBuffer;
		PB::NetWorkPacket mNetWorkPacket;
		class TimerManager * mTimerManager;
		class NetWorkManager * mNetWorkManager;
		class ActionManager * mFunctionManager;		
	private:
		long long mCurrentCorId;
#ifndef _WIN32
		ucontext_t mMainContext;
#else
		void * mMainCoroutineStack;
		std::queue<Coroutine *> mDestoryCoroutine;
#endif
		char mSharedStack[STACK_SIZE];
		std::queue<long long> mResumeCoroutine;
		std::list<CoroutineEvent *> mCorEventList;
		std::unordered_map<long long, Coroutine *> mCoroutineMap;
	};
}