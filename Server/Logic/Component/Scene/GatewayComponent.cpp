#include "GatewayComponent.h"
#include <Object/GameObject.h>
#include <Service/ServiceNode.h>
#include <Scene/RedisComponent.h>
#include <Service/GatewayService.h>
#include <Scene/GameObjectComponent.h>
#include <Service/ServiceNodeComponent.h>
namespace Sentry
{
    bool GatewayComponent::Awake()
    {
        this->mGateService = this->GetComponent<GatewayService>();
        this->mRedisComponent = this->GetComponent<RedisComponent>();
        this->mNodeComponent = this->GetComponent<ServiceNodeComponent>();
        this->mGameObjComponent = this->GetComponent<GameObjectComponent>();
        return true;
    }

    bool GatewayComponent::OnRecvMessage(PacketMapper *msg)
    {
        const std::string &address = msg->GetAddress();
        TcpProxySession *tcpProxySession = this->GetProxySession(address);
        if (tcpProxySession == nullptr)
        {
            return false;
        }
        const std::string &method = msg->GetMethd();
        if (tcpProxySession->IsNodeSession()) // 服务器发送过来的
        {
            if (this->mGateService->HasMethod(method))
            {
                return NetProxyComponent::OnRecvMessage(msg);
            }
            GameObject *gameObject = this->mGameObjComponent->Find(msg->GetUserId());
            if (gameObject != nullptr)
            {
                const std::string &userAddress = gameObject->GetAddress();
                TcpProxySession *userSession = this->GetProxySession(userAddress);
                if (userSession != nullptr)
                {
                    userSession->SendMessageData(msg);
                }
            }
            return true;
        }
        //客户端发送过来的

        if (this->mGateService->HasMethod(method))
        {
            return NetProxyComponent::OnRecvMessage(msg);
        }
        GameObject *gameObject = this->mGameObjComponent->Find(address);
        if (gameObject == nullptr)
        {
            return false;
        }
        const std::string & service = msg->GetService();
        ServiceNode * serviceNode = this->mNodeComponent->GetNodeByServiceName(service);
        if(serviceNode == nullptr)
        {
            return false;
        }
        msg->SetUserId(gameObject->GetId());
        serviceNode->AddMessageToQueue(msg, false);

        return true;
    }
}