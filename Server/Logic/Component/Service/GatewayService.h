//
// Created by zmhy0073 on 2021/10/11.
//

#ifndef GameKeeper_GATEWAYSERVICE_H
#define GameKeeper_GATEWAYSERVICE_H
#include <Service/LocalServiceComponent.h>
namespace GameKeeper
{
    class GatewayService : public LocalServiceComponent
    {
    public:
        GatewayService() { }
        ~GatewayService() { }

    protected:
        bool Awake() override;
    private:
        XCode Login(long long id, c2s::GateLogin_Request & request);
        XCode Logout(long long id, c2s::GateLogout_Request & request);
    private:
        class RedisComponent * mRedisComponent;
        class GatewayComponent * mGateComponent;
        class GameObjectComponent * mGameObjComponent;
    };
}

#endif //GameKeeper_GATEWAYSERVICE_H
