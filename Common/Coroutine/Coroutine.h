#pragma once
#include"CoroutineEvent.h"
namespace Sentry
{
    enum CorState
    {
        Ready,
        Running,
        Suspend,
        Finish,
    };

    class CoroutineManager;

    struct Coroutine
    {
    public:
		Coroutine();
        ~Coroutine();
    public:
        size_t mStackSize;
		Closure * mFunction;
#ifdef SentryAsmCoroutine		
		tb_context_t mCorContext;
#else
		void * mCorContext;	
#ifndef _WIN32	
		ucontext_t mCorContext;
#endif

#endif
        CorState mState;
        long long mCoroutineId;
        CoroutineManager *mScheduler;
    };
};