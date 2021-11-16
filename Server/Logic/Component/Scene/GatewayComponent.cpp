#include "GatewayComponent.h"
#include <Object/GameObject.h>
#include <Service/RpcNodeProxy.h>
#include <Scene/RedisComponent.h>
#include <Service/GatewayService.h>
#include <Scene/GameObjectComponent.h>
#include <Service/NodeProxyComponent.h>
namespace GameKeeper
{
    bool GatewayComponent::Awake()
    {
        this->mGateService = this->GetComponent<GatewayService>();
        this->mRedisComponent = this->GetComponent<RedisComponent>();
        this->mNodeComponent = this->GetComponent<NodeProxyComponent>();
        this->mGameObjComponent = this->GetComponent<GameObjectComponent>();
        return true;
    }

}