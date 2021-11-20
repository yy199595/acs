
#pragma once

#include<Service/LocalServiceComponent.h>
#include<Rpc/ProtoRpcClient.h>

#include <utility>

using namespace com;

namespace GameKeeper
{
    class ServerGroupConfig
    {
    public:
        std::string mName;
        std::string mToken;
        unsigned int mGroupId;
    };
}

namespace GameKeeper
{
    class RpcNodeProxy;
    // 所有方法都注册到这里(全局唯一)
    class NodeCenter : public LocalServiceComponent, public ILoadConfig
    {
    public:
		NodeCenter() = default;
        ~NodeCenter() override = default;

    protected:
        bool Awake() final;

        bool OnLoadConfig() final;
    private:

        XCode Add(const s2s::NodeRegister_Request &nodeInfo, s2s::NodeRegister_Response & response);

        XCode Query(const s2s::NodeQuery_Request & service, s2s::NodeQuery_Response &response);

    private:
        void NoticeAllNode(const s2s::NodeInfo & nodeInfo);
        void AddNewNode(unsigned short areaId, unsigned int nodeId);
        const ServerGroupConfig * GetGroupConfig(unsigned int groupId);
    private:
        class ProtoRpcComponent * mRpcComponent;
        class NodeProxyComponent * mNodeComponent;
        std::unordered_map<unsigned int, ServerGroupConfig> mGroupNodeMap;
        std::unordered_map<unsigned short , std::set<unsigned int>> mServiceNodeMap;
    };
}