#pragma once
#include <string>
#include"CoroutineEvent.h"
namespace GameKeeper
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
        unsigned int mGroupId;
		StaticMethod * mFunction;
#ifdef __COROUTINE_ASM__
		int sid;
		std::string mStack;
		tb_context_t mCorContext;
#elif __linux__
		void * mContextStack;
		ucontext_t mContext;
#elif _WIN32
		void * mContextStack;
#endif
        CorState mState;
        unsigned int mCoroutineId;
    };

	struct Stack
	{
		char * p;
		char * top;
		unsigned int co;
	};
}