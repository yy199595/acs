#pragma once

#include<Component/Component.h>
#include<Other/TimeRecorder.h>
#include <Util/NumberBuilder.h>
namespace Sentry
{
    // 注册本地Lua服务，管理远程回来的回调
    class CallHandler;

	class PacketMapper;
    class CallHandlerComponent : public Component, public IResponseMessageHandler
    {
    public:
        CallHandlerComponent();

        virtual ~CallHandlerComponent() {}

    public:
        bool AddCallHandler(CallHandler * rpcAction, unsigned int & id);
        bool OnResponseMessage(const com::DataPacket_Response & message) final;
    protected:
        bool Awake() override;

    private:
#ifdef _DEBUG
		char mBuffer[100];
#endif
        int mMessageTimeout;
        class TimerComponent *mTimerComponent;
		NumberBuilder<unsigned int> mNumberPool;
		std::unordered_map<unsigned int, shared_ptr<CallHandler>> mRetActionMap;
    private:
        
    };
}