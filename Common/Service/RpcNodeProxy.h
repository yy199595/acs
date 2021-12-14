#pragma once


#include<Object/Object.h>
#include<Async/RpcTask/ProtoRpcTask.h>
#include<Coroutine/Coroutine.h>
#include"Protocol/s2s.pb.h"
using namespace google::protobuf;
namespace GameKeeper
{
    class RpcNodeProxy : public Object
    {
    public:
        explicit RpcNodeProxy(int id);

    public:

        int GetGlobalId() const { return this->mGlobalId; }

        const std::string &GetNodeName() { return this->mNodeName; }

        short GetAreaId() const { return this->mAreaId; }

        short GetNodeId() const { return this->mNodeId; }

        bool UpdateNodeProxy(const s2s::NodeInfo & nodeInfo,long long socketId = 0);

        const s2s::NodeInfo & GetNodeInfo() const { return this->mNodeInfo; }

        bool SendRequestData(const com::Rpc_Request * message);

        com::Rpc_Request * CreateProtoRequest(const std::string & method, int & methodId);
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
        void OnNodeSessionRefresh();
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
        std::queue<const Message *> mWaitSendQueue;
        class RpcConfigComponent *mRpcConfigComponent;
        class ProtoRpcClientComponent * mRpcClientComponent;
    };
}// namespace GameKeeper