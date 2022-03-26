//
// Created by zmhy0073 on 2022/1/13.
//

#ifndef GAMEKEEPER_SERVICENODE_H
#define GAMEKEEPER_SERVICENODE_H
#include"Component/Rpc/RpcClientComponent.h"
namespace Sentry
{
    enum class NodeState
    {
        None,
        Connected, //正在连接
        ConnectFailure, //连接失败
        ConnectSuccessful, //连接成功
    };

    class ProxyClient
    {
    public:
        ProxyClient(const std::string & name, const std::string & address);

    private:
        void SendFromQueue();
    public:
        bool IsConnected();
        NodeState GetState() { return this->mState; }
        const std::string & GetAddress() { return this->mAddress; }
        void PushMessage(std::shared_ptr<com::Rpc_Request> message);
        std::shared_ptr<com::Rpc_Request> PopMessage();
    private:
        bool OnConnectFailure(int count);
    private:
        bool mIsClose;
        NodeState mState;
        int mConnectCount;
        std::string mAddress;
        std::string mServiceName;
        TaskComponent * mTaskComponent;
        RpcClientComponent * mRpcCliemComponent;
        std::shared_ptr<ProtoRpcClient> mNodeClient;
        std::shared_ptr<LoopTaskSource> mLoopTaskSource;
        std::queue<std::shared_ptr<com::Rpc_Request>> mMessageQueue;
    };
}

#endif //GAMEKEEPER_SERVICENODE_H
