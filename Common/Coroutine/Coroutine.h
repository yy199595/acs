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

    class TaskComponent;

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
        void Invoke();
    public:
        int sid;
        Stack mStack;
        CorState mState;
        unsigned int mGroupId;
        tb_context_t mContext;
		StaticMethod * mFunction;
        unsigned int mSwitchCount;
        unsigned int mCoroutineId;
        unsigned int mLastCoroutineId;
    };
}