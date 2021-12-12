
#pragma once

#include<Service/ProtoServiceComponent.h>
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
    class CenterHostService : public ProtoServiceComponent, public ILoadConfig
    {
    public:
		CenterHostService() = default;
        ~CenterHostService() override = default;

    protected:
        bool Awake() final;

        bool LateAwake() final;

        bool OnLoadConfig() final;
    private:

        XCode Add(const s2s::NodeRegister_Request &nodeInfo, s2s::NodeRegister_Response & response);

        XCode Query(const s2s::NodeQuery_Request & service, s2s::NodeQuery_Response &response);

        XCode QueryHosts(s2s::HostQuery_Response & response);
    private:
        void NoticeAllNode(const s2s::NodeInfo & nodeInfo);
        void AddNewNode(unsigned short areaId, unsigned int nodeId);
        const ServerGroupConfig * GetGroupConfig(unsigned int groupId);

    private:
        bool LoadGroupConfig(const std::string & path);
        bool LoadHostConfig(const std::string & path);
    private:
        std::string mHostConfigMd5;
        std::string mGroupConfigMd5;
        std::set<std::string> mServiceHosts;
        class ProtoRpcClientComponent * mRpcComponent;
        class NodeProxyComponent * mNodeComponent;
        std::unordered_map<unsigned int, ServerGroupConfig> mGroupNodeMap;
        std::unordered_map<unsigned short , std::set<unsigned int>> mServiceNodeMap;
    };
}