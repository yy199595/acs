#include "ActionTimeoutTimer.h"
#include <Scene/CallHandlerComponent.h>
#include <Method/CallHandler.h>

namespace Sentry
{
    ActionTimeoutTimer::ActionTimeoutTimer(long long ms, long long callbackId, CallHandlerComponent *mgr)
        : TimerBase(ms)
    {
        this->mActionManager = mgr;
        this->mCallbackId = callbackId;
    }

    bool ActionTimeoutTimer::Invoke()
    {
        // TODO
        return false;
    }
}// namespace Sentry