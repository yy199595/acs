//
// Created by zmhy0073 on 2021/12/22.
//

#ifndef GAMEKEEPER_NODEHELPER_H
#define GAMEKEEPER_NODEHELPER_H
#include"Service/RpcNode.h"
#include"Service/RpcNodeComponent.h"
using namespace GameKeeper;
namespace GameKeeper
{
    class NodeHelper
    {
    public:
        NodeHelper(int nodeId);
        NodeHelper(const std::string & service);

    public:
        XCode Notice(const std::string & func);
        XCode Notice(const std::string & func, const Message & message);
    public:
        XCode Invoke(const std::string & func);
        XCode Invoke(const std::string & func, const Message & message);
    public:
        template<typename T>
        std::shared_ptr<T> Call(const std::string & func);
        template<typename T>
        std::shared_ptr<T> Call(const std::string & func, const Message & message);

    public:
        int GetNodeId() const { return this->mNodeId;}
    private:
        int mNodeId;
        RpcNodeComponent * mNodeComponent;
    };

    template<typename T>
    std::shared_ptr<T> NodeHelper::Call(const std::string &func)
    {
        RpcNode * rpcNode = mNodeComponent->GetServiceNode(mNodeId);
        if(rpcNode == nullptr)
        {
            return nullptr;
        }
        return rpcNode->NewRpcTask(func)->GetData<T>();
    }

    template<typename T>
    std::shared_ptr<T> NodeHelper::Call(const std::string &func, const Message &message)
    {
        RpcNode * rpcNode = mNodeComponent->GetServiceNode(mNodeId);
        if(rpcNode == nullptr)
        {
            return nullptr;
        }
        return rpcNode->NewRpcTask(func, message)->GetData<T>();
    }
}


#endif //GAMEKEEPER_NODEHELPER_H
