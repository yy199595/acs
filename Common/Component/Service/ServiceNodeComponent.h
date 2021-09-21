#pragma once

#include<Component/Component.h>

namespace Sentry
{
    class ServiceNode;

    class ServiceNodeComponent : public Component, public ISecondUpdate
    {
    public:
        ServiceNodeComponent() {}

        ~ServiceNodeComponent() {}

    public:
        bool DelNode(int nodeId);

		virtual int GetPriority() { return 0; }

        bool DelNode(const std::string &address);

		ServiceNode * CreateNode(int areaId, int nodeId, std::string name, std::string address);
	
    protected:
        bool Awake() final;

        void OnSecondUpdate() final;

    public:
        ServiceNode *GetServiceNode(const int nodeId);

        ServiceNode *GetServiceNode(const std::string &address);

        ServiceNode *GetNodeByNodeName(const std::string &nodeName);

        ServiceNode *GetNodeByServiceName(const std::string &service);

    private:
        std::string mCenterIp;
        unsigned short mCenterPort;
        std::list<ServiceNode *> mServiceNodeArray;
        std::unordered_map<int, ServiceNode *> mServiceNodeMap1;
        std::unordered_map<std::string, ServiceNode *> mServiceNodeMap2;
		class ProtocolComponent * mProtocolComponent;
    };
}