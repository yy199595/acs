
#pragma once

#include<Service/LocalServiceComponent.h>
#include<Rpc/RpcClient.h>

using namespace com;

namespace GameKeeper
{
    class RpcNodeProxy;
    // 所有方法都注册到这里(全局唯一)
    class CenterService : public LocalServiceComponent
    {
    public:
		CenterService() = default;
        ~CenterService() override = default;

    protected:
        bool Awake() final;

        void Start() final;

    private:

        XCode Add(const s2s::NodeRegister_Request &nodeInfo, s2s::NodeRegister_Response & response);

        XCode Query(const s2s::NodeQuery_Request & service, s2s::NodeQuery_Response &response);

    private:
        void NoticeAllNode(const s2s::NodeInfo & nodeInfo);
        void AddNewNode(unsigned short areaId, unsigned int nodeId);
    private:
        class NodeProxyComponent * mNodeComponent;
        std::unordered_map<unsigned short , std::set<unsigned int>> mServiceNodeMap;
    };
}