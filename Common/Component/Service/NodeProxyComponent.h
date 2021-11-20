#pragma once

#include<Component/Component.h>

namespace GameKeeper
{
    class RpcNodeProxy;

    class NodeProxyComponent : public Component
    {
    public:
        NodeProxyComponent() = default;
        ~NodeProxyComponent() override = default;

    public:
        bool DelNode(int nodeId);

        int GetPriority() final { return 1; }

        RpcNodeProxy * Create(int uid);

		RpcNodeProxy * CreateNode(int uid, const s2s::NodeInfo & nodeInfo);

    protected:
        bool Awake() final;

        void Start() override;

    public:
        RpcNodeProxy *GetServiceNode(int nodeId);
    private:
		int mAreaId;
        std::string mCenterIp;
        unsigned short mCenterPort;
        class ProtoRpcComponent * mProtocolComponent;
        std::unordered_map<int, RpcNodeProxy *> mServiceNodeMap;
    };
}