//
// Created by zmhy0073 on 2022/1/13.
//
#include"ServiceNode.h"
#include"Util/StringHelper.h"
namespace GameKeeper
{
    ServiceNode::ServiceNode(const std::string &name, const std::string &address)
    {
        this->mIsClose = false;
        this->mAddress = address;
        this->mServiceName = name;
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
                Helper::String::ParseIpAddress(this->mAddress, ip, port);
                while (!clientSession->ConnectAsync(ip, port)->Await())
                {
                    connectCount++;
                    LOG_ERROR("connect ", this->mAddress, " failure count = ", connectCount);
                    this->mTaskComponent->Sleep(3000);
                }
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

    void ServiceNode::PushMessage(std::shared_ptr<com::Rpc_Request> message)
    {
        this->mMessageQueue.emplace(message);
        if(this->mLoopTaskSource != nullptr)
        {
            this->mLoopTaskSource->SetResult();
        }
    }
}
