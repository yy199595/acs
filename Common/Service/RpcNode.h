#pragma once

#include"Protocol/s2s.pb.h"
#include"Async/RpcTask/RpcTaskSource.h"

using namespace google::protobuf;
namespace GameKeeper
{
    class NodeHelper;
    class RpcNode
    {
    public:
        explicit RpcNode(int id);

    public:

        int GetGlobalId() const { return this->mGlobalId; }

        const std::string &GetNodeName() { return this->mNodeName; }

        short GetAreaId() const { return this->mAreaId; }

        short GetNodeId() const { return this->mNodeId; }

        bool UpdateNodeProxy(const s2s::NodeInfo & nodeInfo);

        const s2s::NodeInfo & GetNodeInfo() const { return this->mNodeInfo; }

        std::shared_ptr<com::Rpc_Request> NewRequest(const std::string & method);
    public:
        void Destory();

        bool AddService(const std::string &service);

        bool HasService(const std::string &service);

        void GetServices(std::vector<std::string> &services);

        XCode Call(const std::string & func, std::shared_ptr<RpcTaskSource> taskSource = nullptr);
        XCode Call(const std::string & func, const Message & message, std::shared_ptr<RpcTaskSource> taskSource = nullptr);

    private:
        void ConnectToNode();
        void OnConnectAfter();
    private:
        int mGlobalId;
        short mAreaId;
        short mNodeId;
        std::string mNodeIp;
        std::string mNodeName;        //进程名字
        unsigned short mNodePort;
        s2s::NodeInfo mNodeInfo;
        NodeHelper * mCallHelper;
    private:
        bool mIsClose;
        long long mSocketId;
        TaskComponent *mCorComponent;//协程
        class RpcComponent * mRpcComponent;
        std::set<std::string> mServiceArray;//服务列表
        class RpcConfigComponent *mRpcConfigComponent;
        class RpcClientComponent * mRpcClientComponent;
        std::queue<std::shared_ptr<com::Rpc_Request>> mWaitSendQueue;
    };
}// namespace GameKeeper