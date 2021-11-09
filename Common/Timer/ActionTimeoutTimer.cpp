#include "ActionTimeoutTimer.h"
#include <Scene/RpcResponseComponent.h>
#include <Method/CallHandler.h>

namespace GameKeeper
{
    ActionTimeoutTimer::ActionTimeoutTimer(long long ms, long long callbackId, RpcResponseComponent *mgr)
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
}// namespace GameKeeper