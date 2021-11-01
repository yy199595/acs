#pragma once
#include <Component/Component.h>
namespace GameKeeper
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
}// namespace GameKeeper