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
		MethodProxy * mFunction;
#ifdef __COROUTINE_ASM__	
		char * mStack;
		char * mStackTop;
		tb_context_t mCorContext;
#elif __linux__
		void * mContextStack;
		ucontext_t mCorContext;
#elif _WIN32
		void * mContextStack;
#endif
        CorState mState;		
        long long mCoroutineId;
        CoroutineManager *mScheduler;
    };
};