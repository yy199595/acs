#pragma once
#include<queue>
#include<vector>
#include<functional>
#include"Context/context.h"
#include"CotoutiFunction/CotoutiFunction.h"
#ifndef _WIN32
#if __APPLE__
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#endif
#include<ucontext.h>
#endif
#define SentryAsmCoroutine

#ifdef SentryAsmCoroutine
	#define STACK_SIZE 1024 * 1024
#else
	#ifdef _WIN32
		#define STACK_SIZE 1024 * 2
	#else
		#define STACK_SIZE 1024 * 1024
	#endif
#endif // SentryAsmCoroutine





namespace Sentry
{
    class CoroutineManager;

    typedef std::function<void()> CoroutineAction;

    class CoroutineEvent
    {
    public:
        CoroutineEvent(CoroutineManager *mgr, long long id);

    public:
        virtual bool Invoke() = 0;

    public:
        long long GetBindCoroutineID() { return mBindCoroutineId; }

    protected:
        CoroutineManager *mCorScheduler;
    private:
        long long mBindCoroutineId;
    };

    class CorSleepEvent : public CoroutineEvent
    {
    public:
        CorSleepEvent(CoroutineManager *, long long id, long long ms);

    public:
        bool Invoke() override;

    private:
        long long mNextInvokeTime;
    };

    class CorNextFrameEvent : public CoroutineEvent
    {
    public:
        CorNextFrameEvent(CoroutineManager *, long long id, int count);

    public:
        bool Invoke() override;

    private:
        int mCount;
        int mMaxCount;
    };
}

namespace Sentry
{
	struct Coroutine;
	class CoroutinePool
	{
	public:
		CoroutinePool(int count);
		virtual ~CoroutinePool();
	public:
		Coroutine * Pop();
		void Push(Coroutine * coroutine);
	public:
		Coroutine * Get(unsigned int id);
	private:
		unsigned int mId;
		std::vector<Coroutine *> mAllCoroutine;
		std::queue<unsigned int> mIdleCoroutines;
	};
}