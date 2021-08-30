#pragma once


#include <Object/Object.h>
#include <Component/Service/ServiceNodeComponent.h>
namespace Sentry
{
	class NetMessageProxy;
    class LocalRetActionProxy;

    class ServiceNode : public Object
    {
    public:
        ServiceNode(int areaId, int nodeId, const std::string name, const std::string address);
    public:
        const int GetAreaId() { return this->mNodeInfoMessage.areaid(); }

        const int GetNodeId() { return this->mNodeInfoMessage.nodeid(); }

        const std::string &GetAddress() { return this->mNodeInfoMessage.address(); }

        const std::string &GetNodeName() { return this->mNodeInfoMessage.servername(); }

        const s2s::NodeData_NodeInfo &GetNodeMessage() { return this->mNodeInfoMessage; }

    public:
        bool AddService(const std::string &service);

        bool HasService(const std::string &service);

    public:
        void OnFrameUpdate(float t);
		void OnConnectSuccessful();
    public:
        std::string GetJsonString();

        XCode Notice(const std::string &service, const std::string &method);                        //不回应
        XCode Notice(const std::string &service, const std::string &method, const Message &request);//不回应
    public:
        XCode Invoke(const std::string &service, const std::string &method);

        XCode Invoke(const std::string &service, const std::string &method, const Message &request);

    public:// c++ 使用
        XCode Call(const std::string &service, const std::string &method, Message &response);

        XCode Call(const std::string &service, const std::string &method, const Message &request, Message &response);

	private:
		class TcpProxySession * GetNodeSession();
		XCode SendRpcMessage(NetMessageProxy * message);
		XCode SendRpcMessage(NetMessageProxy * message, Message &response);
    private:
        std::string mIp;
        unsigned short mPort;
        const std::string mAddress;         //监听地址
        const std::string mNodeName;        //进程名字
        std::set<std::string> mServiceArray;//服务列表
        class CoroutineComponent *mCorComponent;//协程
        class SceneActionComponent *mActionManager;
        class SceneNetProxyComponent *mNetWorkManager;
        s2s::NodeData_NodeInfo mNodeInfoMessage;
        ServiceNodeComponent *mServiceNodeManager;
		std::queue<unsigned int> mConnectCoroutines;
        char mSendSharedBuffer[ASIO_TCP_SEND_MAX_COUNT + sizeof(unsigned int)];
    };
}// namespace Sentry