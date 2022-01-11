#pragma once

#include<Component/Component.h>

namespace GameKeeper
{
    class RpcNode;

    class RpcNodeComponent : public Component
    {
    public:
        RpcNodeComponent() = default;
        ~RpcNodeComponent() override = default;

    public:
        bool DelNode(int nodeId);

        int GetPriority() final { return 1; }

        RpcNode * Create(int uid);

		RpcNode * CreateNode(int uid, const s2s::NodeInfo & nodeInfo);

        RpcNode * CreateNode(int uid, const std::string & name, const std::string & ip, unsigned short port);

    protected:
        bool Awake() final;
        bool LateAwake() final;
    public:
        RpcNode *GetServiceNode(int nodeId);

        RpcNode * AllotService(const std::string & name);
    private:
		int mAreaId;
        std::unordered_map<int, RpcNode *> mServiceNodeMap;
    };
}