#pragma once

#include<Component/Component.h>

namespace GameKeeper
{
    class RpcNode;

    class NodeProxyComponent : public Component
    {
    public:
        NodeProxyComponent() = default;
        ~NodeProxyComponent() override = default;

    public:
        bool DelNode(int nodeId);

        int GetPriority() final { return 1; }

        RpcNode * Create(int uid);

		RpcNode * CreateNode(int uid, const s2s::NodeInfo & nodeInfo);

    protected:
        bool Awake() final;
        bool LateAwake() final;
    public:
        RpcNode *GetServiceNode(int nodeId);

        RpcNode * AllotService(const std::string & name);
    private:
		int mAreaId;
        std::string mCenterIp;
        unsigned short mCenterPort;
        std::unordered_map<int, RpcNode *> mServiceNodeMap;
    };
}