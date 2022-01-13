//
// Created by zmhy0073 on 2022/1/13.
//

#ifndef GAMEKEEPER_SERVICENODE_H
#define GAMEKEEPER_SERVICENODE_H
#include"Rpc/RpcClientComponent.h"
namespace GameKeeper
{
    class ServiceNode
    {
    public:
        ServiceNode(const std::string & name, const std::string & address);

    private:
        void SendFromQueue();
    public:
        const std::string & GetAddress() { return this->mAddress; }
        void PushMessage(std::shared_ptr<com::Rpc_Request> message);
    private:
        bool mIsClose;
        std::string mAddress;
        std::string mServiceName;
        TaskComponent * mTaskComponent;
        RpcClientComponent * mRpcCliemComponent;
        std::shared_ptr<LoopTaskSource> mLoopTaskSource;
        std::queue<std::shared_ptr<com::Rpc_Request>> mMessageQueue;
    };
}

#endif //GAMEKEEPER_SERVICENODE_H
