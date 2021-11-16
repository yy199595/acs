#include "NodeProxyComponent.h"

#include <Core/App.h>
#include <Service/RpcNodeProxy.h>
#include <Scene/RpcProtoComponent.h>
#include <Component/Scene/RpcComponent.h>

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

    RpcNodeProxy *NodeProxyComponent::Create(unsigned int uid)
    {
        auto nodeProxy = this->GetServiceNode(uid);
        if (nodeProxy == nullptr)
        {
            nodeProxy = new RpcNodeProxy(uid);
            this->mServiceNodeMap.emplace(uid, nodeProxy);
        }
        return nodeProxy;
    }

	RpcNodeProxy * NodeProxyComponent::CreateNode(unsigned int uid, const s2s::NodeInfo & nodeInfo)
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
		GKAssertRetFalse_F(mProtocolComponent = this->GetComponent<RpcProtoComponent>());
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

	void NodeProxyComponent::OnSecondUpdate()
	{
		auto iter = this->mServiceNodeArray.begin();
		for (; iter != this->mServiceNodeArray.end();)
		{
			RpcNodeProxy *serviceNode = (*iter);
			if (serviceNode == nullptr || !serviceNode->IsActive())
			{
				serviceNode->OnDestory();
				delete serviceNode;
				this->mServiceNodeArray.erase(iter++);
				continue;
			}
			iter++;
		}
	}

    RpcNodeProxy *NodeProxyComponent::GetServiceNode(unsigned int nodeId)
    {
        auto iter = this->mServiceNodeMap.find(nodeId);
        if (iter != this->mServiceNodeMap.end())
        {
            RpcNodeProxy *serviceNode = iter->second;
            if (serviceNode != nullptr && serviceNode->IsActive())
            {
                return serviceNode;
            }
        }
		
        return nullptr;
    }


    RpcNodeProxy *NodeProxyComponent::GetNodeByNodeName(const std::string &nodeName)
    {
        for (RpcNodeProxy *serviceNode : this->mServiceNodeArray)
        {
            if (serviceNode->IsActive() && serviceNode->GetNodeName() == nodeName)
            {
                return serviceNode;
            }
        }
		

        return nullptr;
    }

	RpcNodeProxy *NodeProxyComponent::GetNodeByServiceName(const std::string &service)
    {
        for (RpcNodeProxy *serviceNode: this->mServiceNodeArray)
        {
            if (serviceNode->IsActive() && serviceNode->HasService(service))
            {
                return serviceNode;
            }
        }

        s2s::NodeQuery_Request request;
        s2s::NodeQuery_Response response;
        request.set_areaid(this->mAreaId);
       
        RpcNodeProxy *centerNode = this->GetServiceNode(0);
        XCode code = centerNode->Call("NodeCenter.Query", request, response);

        RpcNodeProxy * newServiceNode = nullptr;
        if (code == XCode::Successful && response.nodeinfos_size() > 0)
        {
            for (int index = 0; index < response.nodeinfos_size(); index++)
            {
                const s2s::NodeInfo & nodeInfo = response.nodeinfos(index);
                unsigned int uid = nodeInfo.areaid() * 10000 + nodeInfo.nodeid();
                newServiceNode = this->CreateNode(uid, nodeInfo);
            }
        }
        return newServiceNode;
    }

}// namespace GameKeeper
