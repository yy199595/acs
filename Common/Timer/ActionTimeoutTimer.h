#pragma once

#include"TimerBase.h"

namespace Sentry
{
    class SceneActionComponent;

    class ActionTimeoutTimer : public TimerBase
    {
    public:
        ActionTimeoutTimer(long long ms, long long callbackId, SceneActionComponent *mgr);

    public:
        bool Invoke() final;

    private:
        long long mCallbackId;
        SceneActionComponent *mActionManager;
    };
}