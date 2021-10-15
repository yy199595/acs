#pragma once

#include<Scene/TcpNetProxyComponent.h>
namespace Sentry
{
    // 网关
    class GatewayComponent : public Component
    {
    public:
        GatewayComponent() {}

        ~GatewayComponent() {}

    protected:
        bool Awake() override;
    private:
        class GatewayService * mGateService;
        class RedisComponent * mRedisComponent;
        class ServiceNodeComponent * mNodeComponent;
        class GameObjectComponent * mGameObjComponent;
    };
}// namespace Sentry