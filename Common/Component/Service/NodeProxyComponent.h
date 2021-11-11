#pragma once

#include<Component/Component.h>

namespace GameKeeper
{
    class NodeProxy;

    class NodeProxyComponent : public Component, public ISecondUpdate
    {
    public:
        NodeProxyComponent() = default;
        ~NodeProxyComponent() override = default;

    public:
        bool DelNode(int nodeId);

        int GetPriority() final { return 1; }

		NodeProxy * CreateNode(int areaId, int nodeId, std::string name, std::string address);
	
    protected:
        bool Awake() final;

        void Start() override;

        void OnSecondUpdate() final;

    public:
        NodeProxy *GetServiceNode(const int nodeId);

        NodeProxy *GetServiceNode(const std::string &address);

        NodeProxy *GetNodeByNodeName(const std::string &nodeName);

        NodeProxy *GetNodeByServiceName(const std::string &service);

    private:
		int mAreaId;
        std::string mCenterIp;
        unsigned short mCenterPort;
        std::list<NodeProxy *> mServiceNodeArray;
        class RpcProtoComponent * mProtocolComponent;
        std::unordered_map<int, NodeProxy *> mServiceNodeMap1;
        std::unordered_map<std::string, NodeProxy *> mServiceNodeMap2;
    };
}