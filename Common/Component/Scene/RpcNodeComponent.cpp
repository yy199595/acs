#include "RpcNodeComponent.h"

#include "Core/App.h"
#include "Service/RpcNode.h"
#include "Component/Rpc/RpcClientComponent.h"

namespace GameKeeper
{
    bool RpcNodeComponent::DelNode(int nodeId)
    {
        auto iter = this->mServiceNodeMap.find(nodeId);
        if (iter != this->mServiceNodeMap.end())
        {
            RpcNode *serviceNode = iter->second;
            mServiceNodeMap.erase(iter);
            if(serviceNode != nullptr)
            {
                serviceNode->Destory();
            }
            return true;
        }
        return false;
    }

    RpcNode *RpcNodeComponent::Create(int uid)
    {
        auto nodeProxy = this->GetServiceNode(uid);
        if (nodeProxy == nullptr)
        {
            nodeProxy = new RpcNode(uid);
            this->mServiceNodeMap.emplace(uid, nodeProxy);
        }
        return nodeProxy;
    }

	RpcNode * RpcNodeComponent::CreateNode(int uid, const s2s::NodeInfo & nodeInfo)
	{
        auto nodeProxy = this->GetServiceNode(uid);
        if(nodeProxy == nullptr)
        {
            nodeProxy = new RpcNode(uid);
        }
        if(!nodeProxy->UpdateNodeProxy(nodeInfo))
        {
            delete nodeProxy;
#ifdef __DEBUG__

#endif
            return nullptr;
        }
        this->mServiceNodeMap[uid] = nodeProxy;
		return nodeProxy;
	}

    bool RpcNodeComponent::Awake()
    {
		const ServerConfig & serverConfig = App::Get().GetConfig();
		LOG_CHECK_RET_FALSE(serverConfig.GetValue("AreaId", this->mAreaId));
		LOG_CHECK_RET_FALSE(serverConfig.GetValue("CenterAddress", "ip", this->mCenterIp));
		LOG_CHECK_RET_FALSE(serverConfig.GetValue("CenterAddress", "port", this->mCenterPort));
        return true;
    }

    bool RpcNodeComponent::LateAwake()
    {
        s2s::NodeInfo centerNodeInfo;
        centerNodeInfo.set_server_name("Center");
        centerNodeInfo.set_server_ip(this->mCenterIp);
        centerNodeInfo.add_services("CenterHostService");
        centerNodeInfo.mutable_listeners()->insert({"rpc", this->mCenterPort});
        this->CreateNode(0, centerNodeInfo);
        return true;
    }

    RpcNode *RpcNodeComponent::GetServiceNode(int nodeId)
    {
        auto iter = this->mServiceNodeMap.find(nodeId);
        return iter != this->mServiceNodeMap.end() ? iter->second : nullptr;
    }

    // 分配服务  TODO
    RpcNode *RpcNodeComponent::AllotService(const string &name)
    {
        auto iter = this->mServiceNodeMap.begin();
        for(; iter != this->mServiceNodeMap.end(); iter++)
        {
            if(iter->second->HasService(name))
            {
                return iter->second;
            }
        }
        return nullptr;
    }
}// namespace GameKeeper