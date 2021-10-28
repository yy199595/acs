#pragma once

#include"TimerBase.h"

namespace Sentry
{
    class CallHandlerComponent;

    class ActionTimeoutTimer : public TimerBase
    {
    public:
        ActionTimeoutTimer(long long ms, long long callbackId, CallHandlerComponent *mgr);

    public:
        bool Invoke() final;

    private:
        long long mCallbackId;
        CallHandlerComponent *mActionManager;
    };
}