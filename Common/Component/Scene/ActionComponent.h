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

	class PacketMapper;
    class ActionComponent : public Component, public IResponseMessageHandler
    {
    public:
        ActionComponent();

        virtual ~ActionComponent() {}

    public:
        unsigned int AddCallback(shared_ptr<LocalRetActionProxy> rpcAction);
        bool OnResponseMessage(const com::DataPacket_Response * message) final;
    protected:
        bool Awake() override;

    private:
#ifdef _DEBUG
		char mBuffer[100];
#endif
        int mMessageTimeout;
        class TimerComponent *mTimerComponent;
		NumberBuilder<unsigned int> mNumberPool;
		std::unordered_map<unsigned int, shared_ptr<LocalRetActionProxy>> mRetActionMap;
    private:
        
    };
}