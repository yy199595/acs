#pragma once

#include<Component/Component.h>

namespace Sentry
{
    class ServiceNode;

    class ServiceNodeComponent : public Component, public IFrameUpdate
    {
    public:
        ServiceNodeComponent() {}

        ~ServiceNodeComponent() {}

    public:
        bool DelNode(int nodeId);

        bool DelNode(const std::string &address);

        bool AddNode(ServiceNode *serviceNode);

    protected:
        bool Awake() final;

        void OnFrameUpdate(float t) final;

    public:
        ServiceNode *GetServiceNode(const int nodeId);

        ServiceNode *GetServiceNode(const std::string &address);

        ServiceNode *GetNodeByNodeName(const std::string &nodeName);

        ServiceNode *GetNodeByServiceName(const std::string &service);

    private:
        std::string mCenterIp;
        unsigned short mCenterPort;
        std::string mCenterAddress;
        class SceneSessionComponent *mNetWorkManager;
        std::list<ServiceNode *> mServiceNodeArray;
        std::unordered_map<int, ServiceNode *> mServiceNodeMap1;
        std::unordered_map<std::string, ServiceNode *> mServiceNodeMap2;
    };
}