#pragma once

#include<Component/Component.h>
#include<Other/TimeRecorder.h>
#include <Util/NumberBuilder.h>
namespace GameKeeper
{
    // 注册本地Lua服务，管理远程回来的回调
    class CallHandler;

	class PacketMapper;
    class CallHandlerComponent : public Component, public IResponseMessageHandler
    {
    public:
        CallHandlerComponent();

        ~CallHandlerComponent() override = default;

    public:
        bool AddCallHandler(CallHandler * rpcAction, unsigned int & id);
        bool OnResponseMessage(const com::DataPacket_Response & message) final;
    protected:
        bool Awake() override;
    private:
        int mMessageTimeout;
        class TimerComponent *mTimerComponent;
		NumberBuilder<unsigned int> mNumberPool;
		std::unordered_map<unsigned int, CallHandler *> mRetActionMap;
    private:
        
    };
}