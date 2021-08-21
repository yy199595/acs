#pragma once
#include<queue>
#include<vector>
#include<functional>

#define __COROUTINE_ASM__

#ifdef __APPLE__
#ifndef __COROUTINE_ASM__
#define __COROUTINE_ASM__
#endif // !1
#endif
#include"CotoutiFunction/CotoutiFunction.h"
#ifdef __COROUTINE_ASM__
	#include"Context/context.h"	
	#define STACK_SIZE 1024 * 1024
#elif __linux__
	#include<ucontext.h>
	#define STACK_SIZE 1024 * 1024
#elif _WIN32
	#define STACK_SIZE 1024 * 1024
#endif

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