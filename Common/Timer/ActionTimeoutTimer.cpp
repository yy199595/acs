#include "ActionTimeoutTimer.h"
#include <Scene/SceneActionComponent.h>
#include <NetWork/NetWorkRetAction.h>

namespace Sentry
{
    ActionTimeoutTimer::ActionTimeoutTimer(long long ms, long long callbackId, SceneActionComponent *mgr)
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