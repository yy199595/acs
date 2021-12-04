#pragma once
#include <string>
#include"CoroutineDef.h"
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

    struct Stack
    {
        char * p;
        char * top;
        size_t size = 0;
		unsigned int co = 0;
    };
	

    class Coroutine
    {
    public:
		explicit Coroutine();
        ~Coroutine();
    public:
        size_t mStackSize;
        unsigned int mGroupId;
		StaticMethod * mFunction;
        unsigned int mSwitchCount;
		unsigned int mLastCoroutineId;
#ifdef __COROUTINE_ASM__
		int sid;
		Stack mStack;
		tb_context_t mContext;
#elif __linux__
		void * mContextStack;
		ucontext_t mContext;
#elif _WIN32
		void * mContextStack;
#endif
        CorState mState;
        unsigned int mCoroutineId;
    };
}