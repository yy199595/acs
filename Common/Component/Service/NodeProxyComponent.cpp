#include "NodeProxyComponent.h"

#include <Core/App.h>
#include <Service/RpcNodeProxy.h>
#include <Scene/ProtoRpcComponent.h>
#include <Component/Scene/ProtoRpcComponent.h>

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
		const ServerConfig & serverConfig = App::Get().GetConfig();
		GKAssertRetFalse_F(serverConfig.GetValue("AreaId", this->mAreaId));
		GKAssertRetFalse_F(serverConfig.GetValue("CenterAddress", "ip", this->mCenterIp));
		GKAssertRetFalse_F(serverConfig.GetValue("CenterAddress", "port", this->mCenterPort));
		GKAssertRetFalse_F(mProtocolComponent = this->GetComponent<ProtoRpcComponent>());
        return true;
    }

    void NodeProxyComponent::Start()
    {
        s2s::NodeInfo centerNodeInfo;
        centerNodeInfo.set_servername("Center");
        centerNodeInfo.set_serverip(this->mCenterIp);
        centerNodeInfo.add_services("NodeCenter");
        centerNodeInfo.mutable_listeners()->insert({"rpc", this->mCenterPort});

        GKAssertRet_F(this->CreateNode(0, centerNodeInfo));
    }

    RpcNodeProxy *NodeProxyComponent::GetServiceNode(int nodeId)
    {
        auto iter = this->mServiceNodeMap.find(nodeId);
        return iter != this->mServiceNodeMap.end() ? iter->second : nullptr;
    }
}// namespace GameKeeper
