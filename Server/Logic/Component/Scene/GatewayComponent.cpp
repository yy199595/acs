#include "GatewayComponent.h"
#include <Object/GameObject.h>
#include <Service/ServiceNode.h>
#include <Scene/RedisComponent.h>
#include <Service/GatewayService.h>
#include <Scene/GameObjectComponent.h>
#include <Service/ServiceNodeComponent.h>
#include <Scene/NodeMaperComponent.h>
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

        if (tcpProxySession->IsNodeSession()) // 服务器发送过来的
        {
            switch(msg->GetMessageType())
            {
                case NetMessageType::S2C_NOTICE:
                case NetMessageType::S2C_REQUEST:
                case NetMessageType::C2S_RESPONSE:
                    this->SendMessagToClient(msg);
                    return true;
                case NetMessageType::S2S_NOTICE:
                case NetMessageType::S2S_REQUEST:
                case NetMessageType::S2S_RESPONSE:
                    return NetProxyComponent::OnRecvMessage(msg);
            }
            return true;
        }
        //客户端发送过来的

        const std::string & service = msg->GetService();
        GameObject *gameObject = this->mGameObjComponent->Find(address);
        if (gameObject == nullptr)
        {
            if(!this->mGateService->HasMethod(service))
            {
                return false;
            }
            return NetProxyComponent::OnRecvMessage(msg);
        }
        NodeMaperComponent * nodeMaperComponent = gameObject->GetComponent<NodeMaperComponent>();
        if(nodeMaperComponent == nullptr)
        {
            return false;
        }

        ServiceNode * serviceNode = nodeMaperComponent->GetService(service);
        if(serviceNode == nullptr)
        {
            return false;
        }
        msg->SetUserId(gameObject->GetId());
        serviceNode->AddMessageToQueue(msg, false);
        return true;
    }

    void GatewayComponent::SendMessagToClient(PacketMapper * msg)
    {
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
    }
}