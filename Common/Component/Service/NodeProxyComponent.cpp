#include "NodeProxyComponent.h"

#include <Core/App.h>
#include <Service/RpcNodeProxy.h>
#include <ProtoRpc/ProtoRpcClientComponent.h>

namespace GameKeeper
{
    bool NodeProxyComponent::DelNode(int nodeId)
    {
        auto iter = this->mServiceNodeMap.find(nodeId);
        if (iter != this->mServiceNodeMap.end())
        {
            RpcNodeProxy *serviceNode = iter->second;
            mServiceNodeMap.erase(iter);
            if(serviceNode != nullptr)
            {
                serviceNode->Destory();
            }
            return true;
        }
        return false;
    }

    RpcNodeProxy *NodeProxyComponent::Create(int uid)
    {
        auto nodeProxy = this->GetServiceNode(uid);
        if (nodeProxy == nullptr)
        {
            nodeProxy = new RpcNodeProxy(uid);
            this->mServiceNodeMap.emplace(uid, nodeProxy);
        }
        return nodeProxy;
    }

	RpcNodeProxy * NodeProxyComponent::CreateNode(int uid, const s2s::NodeInfo & nodeInfo)
	{
        auto nodeProxy = this->GetServiceNode(uid);
        if(nodeProxy == nullptr)
        {
            nodeProxy = new RpcNodeProxy(uid);
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

    bool NodeProxyComponent::Awake()
    {
        this->mProtocolComponent = nullptr;
		const ServerConfig & serverConfig = App::Get().GetConfig();
		LOG_CHECK_RET_FALSE(serverConfig.GetValue("AreaId", this->mAreaId));
		LOG_CHECK_RET_FALSE(serverConfig.GetValue("CenterAddress", "ip", this->mCenterIp));
		LOG_CHECK_RET_FALSE(serverConfig.GetValue("CenterAddress", "port", this->mCenterPort));
        return true;
    }

    bool NodeProxyComponent::LateAwake()
    {
        s2s::NodeInfo centerNodeInfo;
        centerNodeInfo.set_servername("Center");
        centerNodeInfo.set_serverip(this->mCenterIp);
        centerNodeInfo.add_services("CenterHostService");
        centerNodeInfo.mutable_listeners()->insert({"rpc", this->mCenterPort});
        LOG_CHECK_RET_FALSE(mProtocolComponent = this->GetComponent<ProtoRpcClientComponent>());

        this->CreateNode(0, centerNodeInfo);
        return true;
    }

    RpcNodeProxy *NodeProxyComponent::GetServiceNode(int nodeId)
    {
        auto iter = this->mServiceNodeMap.find(nodeId);
        return iter != this->mServiceNodeMap.end() ? iter->second : nullptr;
    }

    // 分配服务  TODO
    RpcNodeProxy *NodeProxyComponent::AllotService(const string &name)
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
