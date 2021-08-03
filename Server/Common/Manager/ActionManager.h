#pragma once

#include<Manager/Manager.h>
#include<Other/TimeRecorder.h>

namespace Sentry
{
    // 注册本地Lua服务，管理远程回来的回调
    class NetLuaAction;

    class LocalActionProxy;

    class LocalRetActionProxy;

    class ActionManager : public Manager
    {
    public:
        ActionManager();

        virtual ~ActionManager() {}

    public:
        bool InvokeCallback(NetMessageProxy *messageData);

        long long AddCallback(shared_ptr<LocalRetActionProxy> rpcAction);

    protected:
        bool OnInit() override;

    private:
        int mMessageTimeout;
        std::string mMessageBuffer;
        TimeRecorder mLogicTimeRecorder;
        TimeRecorder mNetLatencyRecorder;

        class TimerManager *mTimerManager;

    private:
        std::unordered_map<long long, shared_ptr<LocalRetActionProxy>> mRetActionMap;
    };
}