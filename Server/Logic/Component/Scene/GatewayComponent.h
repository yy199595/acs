#pragma once

#include<Scene/NetProxyComponent.h>
namespace Sentry
{
    // 网关
    class GatewayComponent : public NetProxyComponent
    {
    public:
        GatewayComponent() {}

        ~GatewayComponent() {}

    protected:
        bool Awake() override;
        bool OnRecvMessage(PacketMapper *msg) override;

    private:
        class GatewayService * mGateService;
        class RedisComponent * mRedisComponent;
        class ServiceNodeComponent * mNodeComponent;
        class GameObjectComponent * mGameObjComponent;
    };
}// namespace Sentry