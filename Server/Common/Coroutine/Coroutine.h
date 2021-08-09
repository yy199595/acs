#pragma once

#include"CoroutineEvent.h"

#ifndef _WIN32
#if __APPLE__
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#endif
#include<ucontext.h>
#else

#include <windows.h>

#endif

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
        Coroutine() {}

        ~Coroutine();

    public:
        size_t mStackSize;
        void *mContextStack;
#ifndef _WIN32
        ucontext_t mCorContext;
#endif
        CorState mState;
        long long mCoroutineId;
        CoroutineAction mBindFunc;
        CoroutineManager *mScheduler;
    };
}