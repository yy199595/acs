#pragma once

#include"Protocol/s2s.pb.h"
#include<Async/RpcTask/ProtoRpcTask.h>

using namespace google::protobuf;
namespace GameKeeper
{
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

        bool SendRequestData(com::Rpc_Request * message);

        com::Rpc_Request * NewRequest(const std::string & method, int & methodId);
    public:
        void Destory();

        bool AddService(const std::string &service);

        bool HasService(const std::string &service);

        void GetServices(std::vector<std::string> &services);

    public:

        XCode Notice(const std::string &method);                        //不回应
        XCode Notice(const std::string &method, const Message &request);//不回应

        XCode Call(const std::string & func);
        XCode Call(const std::string & func, const Message & request);
        XCode Call(const std::string & func, std::shared_ptr<Message> response);
        XCode Call(const std::string & func, const Message & request, std::shared_ptr<Message> response);
    public:

       std::shared_ptr<CppProtoRpcTask> NewRpcTask(const std::string & method);

       std::shared_ptr<CppProtoRpcTask> NewRpcTask(const std::string & method, const Message & message);

    private:
        void OnConnectAfter();
        class ProtoRpcClient *GetTcpSession();
    private:
        int mGlobalId;
        short mAreaId;
        short mNodeId;
        std::string mNodeIp;
        std::string mNodeName;        //进程名字
        unsigned short mNodePort;
        s2s::NodeInfo mNodeInfo;

    private:
        bool mIsClose;
        long long mSocketId;

        std::set<std::string> mServiceArray;//服务列表
        TaskComponent *mCorComponent;//协程
        class ProtoRpcComponent * mRpcComponent;
        class RpcConfigComponent *mRpcConfigComponent;
        std::queue<com::Rpc_Request *> mWaitSendQueue;
        class ProtoRpcClientComponent * mRpcClientComponent;
    };
}// namespace GameKeeper