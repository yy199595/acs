#pragma once

#include<Component/Component.h>
#include<Other/TimeRecorder.h>
#include <Util/NumberBuilder.h>
namespace GameKeeper
{
    // 注册本地Lua服务，管理远程回来的回调
    class CallHandler;

	class PacketMapper;
    class RpcResponseComponent : public Component, public IProtoResponse
    {
    public:
        RpcResponseComponent();

        ~RpcResponseComponent() override = default;

    public:
		const CallHandler * GetRpcHandler(unsigned int id) const;
		bool OnResponse(const com::Rpc_Response & message) final;
        bool AddCallHandler(CallHandler * rpcAction, unsigned int & id);	
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