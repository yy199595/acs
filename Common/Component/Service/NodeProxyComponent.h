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
        bool LateAwake() final;
    public:
        RpcNodeProxy *GetServiceNode(int nodeId);

        RpcNodeProxy * AllotService(const std::string & name);
    private:
		int mAreaId;
        std::string mCenterIp;
        unsigned short mCenterPort;
        class ProtoRpcClientComponent * mProtocolComponent;
        std::unordered_map<int, RpcNodeProxy *> mServiceNodeMap;
    };
}