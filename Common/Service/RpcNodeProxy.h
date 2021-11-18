#pragma once


#include <Object/Object.h>
#include <Service/NodeProxyComponent.h>
namespace GameKeeper
{
    class CallHandler;
    class RpcNodeProxy : public Object
    {
    public:
        explicit RpcNodeProxy(unsigned int id);

    public:

        unsigned int GetGlobalId() const { return this->mGlobalId; }

        const std::string &GetNodeName() { return this->mNodeName; }

        unsigned short GetAreaId() const { return this->mAreaId; }

        unsigned short GetNodeId() const { return this->mNodeId; }

        bool UpdateNodeProxy(const s2s::NodeInfo & nodeInfo,long long socketId = 0);

        const s2s::NodeInfo & GetNodeInfo() const { return this->mNodeInfo; }
    public:
        void Destory();

        bool AddService(const std::string &service);

        bool HasService(const std::string &service);

        void GetServicers(std::vector<std::string> &services);

    public:

        XCode Notice(const std::string &method);                        //不回应
        XCode Notice(const std::string &method, const Message &request);//不回应
    public:
        XCode Invoke(const std::string &method);

        XCode Invoke(const std::string &method, const Message &request);

    public:// c++ 使用
        XCode Call(const std::string &method, Message &response);

        XCode Call(const std::string &method, const Message &request, Message &response);

    private:
        void OnNodeSessionRefresh();
        class RpcClient *GetTcpSession();
		bool AddRequestDataToQueue(const com::Rpc_Request * message);
		com::Rpc_Request * CreateRequest(const std::string & method);
    private:
        std::string mNodeIp;
        std::string mAddress;
        unsigned int mGlobalId;
        std::string mNodeName;        //进程名字
        unsigned short mAreaId;
        unsigned short mNodeId;
        unsigned short mNodePort;
        s2s::NodeInfo mNodeInfo;

    private:
        bool mIsClose;
        long long mSocketId;
        class RpcComponent * mRpcComponent;
        std::set<std::string> mServiceArray;//服务列表
        class CoroutineComponent *mCorComponent;//协程
        std::queue<unsigned int> mCoroutines;
        std::queue<const Message *> mWaitSendQueue;
        class RpcProtoComponent *mProtocolComponent;
        class RpcResponseComponent *mResponseComponent;
    };
}// namespace GameKeeper