#pragma once


#include <Object/Object.h>
#include <Component/Service/NodeProxyComponent.h>
namespace GameKeeper
{
    class CallHandler;
    class NodeProxy : public Object
    {
    public:
        NodeProxy(int areaId, int nodeId, const std::string & name, const std::string & address, long long socketId = 0);

    public:

        const std::string &GetAddress() { return this->mAddress; }

        const std::string &GetNodeName() { return this->mNodeName; }

        int GetAreaId() const { return this->mAreaId; }

        int GetNodeId() const { return this->mNodeId; }

        int GetNodeUId() const { return this->mAreaId * 10000 + this->mNodeId; }

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
        void LoopSendMessage();
        class RpcConnector *GetTcpSession();
		void AddRequestDataToQueue(const com::Rpc_Request * message);
		com::Rpc_Request * CreateRequest(const std::string & method);
    private:
        std::string mMessageBuffer;
    private:
        int mAreaId;
        int mNodeId;
        bool mIsClose;
        std::string mIp;
        long long mSocketId;
        unsigned int mCorId;
        unsigned short mPort;

        const std::string mAddress;         //监听地址
        const std::string mNodeName;        //进程名字
        std::set<std::string> mServiceArray;//服务列表
        class CoroutineComponent *mCorComponent;//协程
        std::queue<unsigned int> mCoroutines;
        class RpcComponent * mRpcComponent;
        std::queue<const Message *> mWaitSendQueue;
        class RpcProtoComponent *mProtocolComponent;
        class RpcResponseComponent *mResponseComponent;
    };
}// namespace GameKeeper