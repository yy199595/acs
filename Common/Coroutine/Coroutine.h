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
        int sid;
        Stack mStack;
        CorState mState;
        size_t mStackSize;
        unsigned int mGroupId;
        tb_context_t mContext;
		StaticMethod * mFunction;
        unsigned int mSwitchCount;
        unsigned int mCoroutineId;
        unsigned int mLastCoroutineId;
    };
}