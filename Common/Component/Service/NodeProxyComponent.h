#pragma once

#include<Component/Component.h>

namespace GameKeeper
{
    class RpcNodeProxy;

    class NodeProxyComponent : public Component, public ISecondUpdate
    {
    public:
        NodeProxyComponent() = default;
        ~NodeProxyComponent() override = default;

    public:
        bool DelNode(int nodeId);

        int GetPriority() final { return 1; }

        RpcNodeProxy * Create(unsigned int uid);

		RpcNodeProxy * CreateNode(unsigned int uid, std::string name, std::string address);
	
    protected:
        bool Awake() final;

        void Start() override;

        void OnSecondUpdate() final;

    public:
        RpcNodeProxy *GetServiceNode(unsigned int nodeId);

        RpcNodeProxy *GetServiceNode(const std::string &address);

        RpcNodeProxy *GetNodeByNodeName(const std::string &nodeName);

        RpcNodeProxy *GetNodeByServiceName(const std::string &service);

    private:
		int mAreaId;
        std::string mCenterIp;
        unsigned short mCenterPort;
        std::list<RpcNodeProxy *> mServiceNodeArray;
        class RpcProtoComponent * mProtocolComponent;
        std::unordered_map<int, RpcNodeProxy *> mServiceNodeMap1;
        std::unordered_map<std::string, RpcNodeProxy *> mServiceNodeMap2;
    };
}