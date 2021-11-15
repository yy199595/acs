#include "NodeProxyComponent.h"

#include <Core/App.h>
#include <Service/RpcNodeProxy.h>
#include <Scene/RpcProtoComponent.h>
#include <Component/Scene/RpcComponent.h>

namespace GameKeeper
{
    bool NodeProxyComponent::DelNode(int nodeId)
    {
        auto iter = this->mServiceNodeMap1.find(nodeId);
        if (iter != this->mServiceNodeMap1.end())
        {
            RpcNodeProxy *serviceNode = iter->second;
            if (serviceNode != nullptr)
            {
                serviceNode->SetActive(false);
                const std::string &address = serviceNode->GetAddress();
                auto iter1 = this->mServiceNodeMap2.find(address);
                if (iter1 != this->mServiceNodeMap2.end())
                {
                    this->mServiceNodeMap2.erase(iter1);
                }
            }
            this->mServiceNodeMap1.erase(iter);
            return true;
        }
        return false;
    }

	RpcNodeProxy * NodeProxyComponent::CreateNode(unsigned int uid, std::string name, std::string address)
	{
		auto serviceNode = new RpcNodeProxy(uid, name, address);
		if (serviceNode != nullptr)
		{
			serviceNode->Init(name);
			this->mServiceNodeArray.push_back(serviceNode);
			this->mServiceNodeMap2.emplace(address, serviceNode);
            this->mServiceNodeMap1.emplace(serviceNode->GetNodeUId(), serviceNode);
            GKDebugInfo("create new service " << name << "  [" << address << "]");
		}
		return serviceNode;
	}

    bool NodeProxyComponent::Awake()
    {
		ServerConfig & ServerCfg = App::Get().GetConfig();
		GKAssertRetFalse_F(ServerCfg.GetValue("AreaId", this->mAreaId));
		GKAssertRetFalse_F(ServerCfg.GetValue("CenterAddress", "ip", this->mCenterIp));
		GKAssertRetFalse_F(ServerCfg.GetValue("CenterAddress", "port", this->mCenterPort));
		GKAssertRetFalse_F(mProtocolComponent = this->GetComponent<RpcProtoComponent>());
        return true;
    }

    void NodeProxyComponent::Start()
    {
        const std::string centerAddress = this->mCenterIp + ":" + std::to_string(this->mCenterPort);
        this->CreateNode(0, "Center", centerAddress)->AddService("CenterService");
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
        auto iter = this->mServiceNodeMap1.find(nodeId);
        if (iter != this->mServiceNodeMap1.end())
        {
            RpcNodeProxy *serviceNode = iter->second;
            if (serviceNode != nullptr && serviceNode->IsActive())
            {
                return serviceNode;
            }
        }
		
        return nullptr;
    }

    RpcNodeProxy *NodeProxyComponent::GetServiceNode(const std::string &address)
    {
        auto iter = this->mServiceNodeMap2.find(address);
        if (iter != this->mServiceNodeMap2.end())
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
        XCode code = centerNode->Call("CenterService.Query", request, response);

        RpcNodeProxy * newServiceNode = nullptr;
        if (code == XCode::Successful && response.nodeinfos_size() > 0)
        {
            for (int index = 0; index < response.nodeinfos_size(); index++)
            {
                const int uid = response.nodeinfos(index).uid();
                const std::string &address = response.nodeinfos(index).address();
                const std::string &nodeName = response.nodeinfos(index).servername();
                newServiceNode = this->CreateNode(uid, nodeName, address);

            }
        }
        return newServiceNode;
    }

}// namespace GameKeeper
