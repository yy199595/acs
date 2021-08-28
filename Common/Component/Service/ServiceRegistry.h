
#pragma once

#include<Service/LocalService.h>
#include<NetWork/TcpClientSession.h>

using namespace com;

namespace Sentry
{
    class ServiceNode;

    struct ActionProxyInfo
    {
    public:
        int mAreaId;                  //服务器组id
        std::string mActionName;      //action名字
        std::string mListenerAddress; //监听地址
    public:
        bool operator==(ActionProxyInfo &actionInfo)
        {
            return this->mAreaId == actionInfo.mAreaId && this->mActionName == actionInfo.mActionName &&
                   this->mListenerAddress == actionInfo.mListenerAddress;
        }
    };

    class ProxyService;

    class TcpSessionListener;

    // 所有方法都注册到这里(全局唯一)
    class ServiceRegistry : public LocalService
    {
    public:
        ServiceRegistry();

        ~ServiceRegistry() {}

    protected:
        bool Awake() final;

        void Start() final;

    private:
        void NoticeNode(int areaId);

        XCode RegisterNode(long long id, const s2s::NodeRegister_Request &nodeInfo);

        XCode QueryNodes(long long id, const com::Int32Data &areaId, s2s::NodeData_Array &nodeArray);

    private:
        class SceneSessionComponent *mNetWorkManager;

        shared_ptr<TcpSessionListener> mTcpSessionListener;
        std::unordered_map<long long, ServiceNode *> mServiceNodeMap;
    };
}