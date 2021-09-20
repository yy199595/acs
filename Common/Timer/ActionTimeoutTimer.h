#pragma once

#include"TimerBase.h"

namespace Sentry
{
    class ActionComponent;

    class ActionTimeoutTimer : public TimerBase
    {
    public:
        ActionTimeoutTimer(long long ms, long long callbackId, ActionComponent *mgr);

    public:
        bool Invoke() final;

    private:
        long long mCallbackId;
        ActionComponent *mActionManager;
    };
}