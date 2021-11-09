#pragma once

#include"TimerBase.h"

namespace GameKeeper
{
    class RpcResponseComponent;

    class ActionTimeoutTimer : public TimerBase
    {
    public:
        ActionTimeoutTimer(long long ms, long long callbackId, RpcResponseComponent *mgr);

    public:
        bool Invoke() final;

    private:
        long long mCallbackId;
        RpcResponseComponent *mActionManager;
    };
}