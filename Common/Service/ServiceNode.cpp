//
// Created by zmhy0073 on 2022/1/13.
//
#include"ServiceNode.h"
#include"Util/StringHelper.h"
#include"Service/RedisService.h"
#include"Scene/ServiceComponent.h"
#include"Service/ServiceEntity.h"
namespace GameKeeper
{
    ServiceNode::ServiceNode(const std::string &name, const std::string &address)
    {
        this->mIsClose = false;
        this->mAddress = address;
        this->mServiceName = name;
        this->mState = NodeState::None;
        this->mTaskComponent = App::Get().GetTaskComponent();
        this->mTaskComponent->Start(&ServiceNode::SendFromQueue, this);
        this->mRpcCliemComponent = App::Get().GetComponent<RpcClientComponent>();
    }

    void ServiceNode::SendFromQueue()
    {
        this->mLoopTaskSource = std::make_shared<LoopTaskSource>();
        auto clientSession = this->mRpcCliemComponent->MakeSession(this->mServiceName, this->mAddress);
        while (!this->mIsClose)
        {
            if (this->mMessageQueue.empty())
            {
                this->mLoopTaskSource->Await();
            }
            if (!clientSession->IsOpen())
            {
                std::string ip;
                int connectCount = 0;
                unsigned short port = 0;
                this->mState = NodeState::Connected;
                Helper::String::ParseIpAddress(this->mAddress, ip, port);
                while (!clientSession->ConnectAsync(ip, port)->Await())
                {
                    connectCount++;
                    LOG_ERROR("connect ", this->mAddress, " failure count = ", connectCount);
                    if(!this->OnConnectFailure(connectCount))
                    {
                        return;
                    }
                    this->mTaskComponent->Sleep(5000);
                }
                this->mState = NodeState::ConnectSuccessful;
            }
            while (!this->mMessageQueue.empty())
            {
                auto requestMessage = this->mMessageQueue.front();
                this->mMessageQueue.pop();
                clientSession->SendToServer(requestMessage);
            }
        }
        long long id = clientSession->GetSocketId();
        this->mRpcCliemComponent->StartClose(id);
    }

    bool ServiceNode::OnConnectFailure(int count)
    {
        if (count <= 3)
        {
            return true;
        }
        RedisService *redisService = App::Get().GetComponent<RedisService>();
        ServiceComponent *serviceComponent = App::Get().GetComponent<ServiceComponent>();
        redisService->RemoveNode(this->mAddress);
        auto serviceEntity = serviceComponent->GetServiceEntity(this->mServiceName);
        if(serviceEntity != nullptr)
        {
            return serviceEntity->RemoveAddress(this->mAddress);
        }
        return false;
    }

    std::shared_ptr<com::Rpc_Request> ServiceNode::PopMessage()
    {
        if(this->mMessageQueue.empty())
        {
            return nullptr;
        }
        auto res = this->mMessageQueue.front();
        this->mMessageQueue.pop();
        return res;
    }

    void ServiceNode::PushMessage(std::shared_ptr<com::Rpc_Request> message)
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
