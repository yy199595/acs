﻿
#pragma once

#include"Component/ServiceBase/ServiceComponent.h"
#include"Network/Rpc/ProtoRpcClient.h"

#include <utility>

using namespace com;

namespace GameKeeper
{
    class RpcNode;
    // 所有方法都注册到这里(全局唯一)
    class CenterHostService : public ServiceComponent, public ILoadConfig
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
        XCode NoticeAllNode(const s2s::NodeInfo & nodeInfo);
        void AddNewNode(unsigned short areaId, unsigned int nodeId);
    private:
        std::set<std::string> mServiceHosts;
        class RpcNodeComponent * mNodeComponent;
        class RpcClientComponent * mRpcComponent;
        std::unordered_map<unsigned short , std::set<unsigned int>> mServiceNodeMap;
    };
}