//
// Created by zmhy0073 on 2022/1/13.
//
#include"ProxyClient.h"
#include"Util/StringHelper.h"
#include"Service/NodeAddressService.h"
#include"Scene/ServiceProxyComponent.h"
#include"Service/ServiceProxy.h"
namespace Sentry
{
    ProxyClient::ProxyClient(const std::string &name, const std::string &address)
    {
        this->mIsClose = false;
        this->mAddress = address;
        this->mServiceName = name;
        this->mState = NodeState::None;
        this->mTaskComponent = App::Get().GetTaskComponent();
        this->mTaskComponent->Start(&ProxyClient::SendFromQueue, this);
        this->mRpcCliemComponent = App::Get().GetComponent<RpcClientComponent>();
    }

    bool ProxyClient::IsConnected()
    {
        if (this->mNodeClient == nullptr)
        {
            this->mNodeClient = this->mRpcCliemComponent->MakeSession
                    (this->mServiceName, this->mAddress);
        }
        if (this->mNodeClient->IsOpen())
        {
            return true;
        }
        std::string ip;
        unsigned short port = 0;
        Helper::String::ParseIpAddress(this->mAddress, ip, port);
        for (int index = 0; index < 3; index++)
        {
            if(this->mNodeClient->ConnectAsync(ip, port)->Await())
            {
                return true;
            }
            this->mTaskComponent->Sleep(1000);
            LOG_ERROR("connect ", this->mAddress, " failure count = ", index);
        }
        return false;
    }

    void ProxyClient::SendFromQueue()
    {
        this->mLoopTaskSource = std::make_shared<LoopTaskSource>();
        while (!this->mIsClose)
        {
            if (this->mMessageQueue.empty())
            {
                this->mLoopTaskSource->Await();
            }
            if (!this->IsConnected())
            {
                return;
            }
            while (!this->mMessageQueue.empty())
            {
                auto requestMessage = this->mMessageQueue.front();
                this->mMessageQueue.pop();
                this->mNodeClient->SendToServer(requestMessage);
            }
        }
        long long id = this->mNodeClient->GetSocketId();
        this->mRpcCliemComponent->StartClose(id);
    }

    bool ProxyClient::OnConnectFailure(int count)
    {
        if (count <= 3)
        {
            return true;
        }
        NodeAddressService *redisService = App::Get().GetComponent<NodeAddressService>();
        ServiceProxyComponent *serviceComponent = App::Get().GetComponent<ServiceProxyComponent>();
        redisService->RemoveNode(this->mAddress);
        auto serviceEntity = serviceComponent->GetServiceProxy(this->mServiceName);
        if(serviceEntity != nullptr)
        {
            return serviceEntity->RemoveAddress(this->mAddress);
        }
        return false;
    }

    std::shared_ptr<com::Rpc_Request> ProxyClient::PopMessage()
    {
        if(this->mMessageQueue.empty())
        {
            return nullptr;
        }
        auto res = this->mMessageQueue.front();
        this->mMessageQueue.pop();
        return res;
    }

    void ProxyClient::PushMessage(std::shared_ptr<com::Rpc_Request> message)
    {
        this->mMessageQueue.emplace(message);
#ifdef __DEBUG__
        LOG_WARN("push to [", this->mAddress, "]");
#endif
        if(this->mLoopTaskSource != nullptr)
        {
            this->mLoopTaskSource->SetResult();
        }
    }
}
