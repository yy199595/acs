#pragma once

#include<Component/Component.h>
#include<Other/TimeRecorder.h>
#include <Util/NumberBuilder.h>
namespace Sentry
{
    // 注册本地Lua服务，管理远程回来的回调
    class NetLuaAction;

    class LocalActionProxy;

    class LocalRetActionProxy;

	class NetMessageProxy;
    class SceneActionComponent : public Component
    {
    public:
        SceneActionComponent();

        virtual ~SceneActionComponent() {}

    public:
        bool InvokeCallback(NetMessageProxy *messageData);
        unsigned int AddCallback(shared_ptr<LocalRetActionProxy> rpcAction);

    protected:
        bool Awake() override;

    private:
        int mMessageTimeout;
        std::string mMessageBuffer;
        TimeRecorder mLogicTimeRecorder;
        TimeRecorder mNetLatencyRecorder;
        class TimerComponent *mTimerComponent;
		NumberBuilder<unsigned int> mNumberPool;
		std::unordered_map<unsigned int, shared_ptr<LocalRetActionProxy>> mRetActionMap;
    private:
        
    };
}