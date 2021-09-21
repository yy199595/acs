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

    class CoroutineComponent;

	

    struct Coroutine
    {
    public:
		Coroutine();
        ~Coroutine();
    public:
        size_t mStackSize;
		MethodProxy * mFunction;
#ifdef __COROUTINE_ASM__
		int sid;
		char * mStackTop;
		std::string mStack;
		tb_context_t mCorContext;
#elif __linux__
		void * mContextStack;
		ucontext_t mCorContext;
#elif _WIN32
		void * mContextStack;
#endif
        CorState mState;		
        long long mCoroutineId;
    };

	struct Stack
	{
		char * p;
		char * top;
		Coroutine * co;
	};
	

};