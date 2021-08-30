
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
    class ServiceCenter : public LocalService
    {
    public:
        ServiceCenter();

        ~ServiceCenter() {}

    protected:
        bool Awake() final;

        void Start() final;

    private:
        void NoticeNode(int areaId);

        XCode Add(long long id, const s2s::NodeRegister_Request &nodeInfo);

        XCode Query(long long id, const com::Int32Data &areaId, s2s::NodeData_Array &nodeArray);

    private:
        std::unordered_map<long long, ServiceNode *> mServiceNodeMap;
    };
}