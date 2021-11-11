#pragma once
#include <Component/Component.h>
namespace GameKeeper
{
    // 网关
    class GatewayComponent : public Component
    {
    public:
        GatewayComponent() = default;

        ~GatewayComponent() final = default;

    protected:
        bool Awake() override;
    private:
        class GatewayService * mGateService;
        class RedisComponent * mRedisComponent;
        class NodeProxyComponent * mNodeComponent;
        class GameObjectComponent * mGameObjComponent;
    };
}// namespace GameKeeper