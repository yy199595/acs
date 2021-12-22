//
// Created by zmhy0073 on 2021/12/22.
//

#include"NodeHelper.h"
#include"Core/App.h"
namespace GameKeeper
{
    NodeHelper::NodeHelper(int nodeId)
        : mNodeId(nodeId)
    {
        this->mNodeComponent = App::Get().GetComponent<RpcNodeComponent>();
    }

    NodeHelper::NodeHelper(const std::string &service)
        : mNodeId(0)
    {
        this->mNodeComponent = App::Get().GetComponent<RpcNodeComponent>();
        RpcNode * rpcNode = this->mNodeComponent->AllotService(service);
        if(rpcNode == nullptr)
        {
            this->mNodeId = rpcNode->GetGlobalId();
        }
    }

    XCode NodeHelper::Notice(const std::string &func)
    {
        RpcNode * rpcNode = this->mNodeComponent->GetServiceNode(this->mNodeId);
        if(rpcNode == nullptr)
        {
            return XCode::CallServiceNotFound;
        }
        rpcNode->NewRequest(func);
        return XCode::Successful;
    }

    XCode NodeHelper::Notice(const std::string &func, const Message &message)
    {
        RpcNode *rpcNode = this->mNodeComponent->GetServiceNode(this->mNodeId);
        if (rpcNode == nullptr)
        {
            return XCode::CallServiceNotFound;
        }
        auto requestData = rpcNode->NewRequest(func);
        requestData->mutable_data()->PackFrom(message);
        return XCode::Successful;
    }

    XCode NodeHelper::Invoke(const std::string &func)
    {
        RpcNode * rpcNode = this->mNodeComponent->GetServiceNode(this->mNodeId);
        if(rpcNode == nullptr)
        {
            return XCode::CallServiceNotFound;
        }
        return rpcNode->NewRpcTask(func)->GetCode();
    }

    XCode NodeHelper::Invoke(const std::string &func, const Message &message)
    {
        RpcNode * rpcNode = this->mNodeComponent->GetServiceNode(this->mNodeId);
        if(rpcNode == nullptr)
        {
            return XCode::CallServiceNotFound;
        }
        return rpcNode->NewRpcTask(func, message)->GetCode();
    }
}