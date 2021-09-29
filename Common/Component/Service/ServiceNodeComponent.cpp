#include "ServiceNodeComponent.h"

#include <Core/App.h>
#include <Service/ServiceNode.h>
#include <Scene/NetSessionComponent.h>
#include <Scene/ProtocolComponent.h>
namespace Sentry
{
    bool ServiceNodeComponent::DelNode(int nodeId)
    {
        auto iter = this->mServiceNodeMap1.find(nodeId);
        if (iter != this->mServiceNodeMap1.end())
        {
            ServiceNode *serviceNode = iter->second;
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

	ServiceNode * ServiceNodeComponent::CreateNode(int uid, std::string name, std::string address)
	{
		auto iter = this->mServiceNodeMap1.find(uid);
		if (iter != this->mServiceNodeMap1.end())
		{
			return iter->second;
		}
		ServiceNode * serviceNode = new ServiceNode(uid, name, address);
		if (serviceNode != nullptr)
		{
			serviceNode->Init(name);
			this->mServiceNodeArray.push_back(serviceNode);
			this->mServiceNodeMap1.emplace(uid, serviceNode);
			this->mServiceNodeMap2.emplace(address, serviceNode);
			SayNoDebugInfo("create new service " << name << "  [" << address << "]");
		}
		return serviceNode;
	}

    bool ServiceNodeComponent::Awake()
    {
		ServerConfig & ServerCfg = App::Get().GetConfig();
		SayNoAssertRetFalse_F(ServerCfg.GetValue("NodeId", this->mNodeId));
		SayNoAssertRetFalse_F(ServerCfg.GetValue("CenterAddress", "ip", this->mCenterIp));
		SayNoAssertRetFalse_F(ServerCfg.GetValue("CenterAddress", "port", this->mCenterPort));
		SayNoAssertRetFalse_F(mProtocolComponent = Scene::GetComponent<ProtocolComponent>());
        std::string centerAddress = this->mCenterIp + ":" + std::to_string(this->mCenterPort);       	
		return this->CreateNode(0, "Center", centerAddress)->AddService("CenterService");
    }

	void ServiceNodeComponent::OnSecondUpdate()
	{
		auto iter = this->mServiceNodeArray.begin();
		for (; iter != this->mServiceNodeArray.end();)
		{
			ServiceNode *serviceNode = (*iter);
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

    ServiceNode *ServiceNodeComponent::GetServiceNode(const int nodeId)
    {
        auto iter = this->mServiceNodeMap1.find(nodeId);
        if (iter != this->mServiceNodeMap1.end())
        {
            ServiceNode *serviceNode = iter->second;
            if (serviceNode != nullptr && serviceNode->IsActive())
            {
                return serviceNode;
            }
        }
		
        return nullptr;
    }

    ServiceNode *ServiceNodeComponent::GetServiceNode(const std::string &address)
    {
        auto iter = this->mServiceNodeMap2.find(address);
        if (iter != this->mServiceNodeMap2.end())
        {
            ServiceNode *serviceNode = iter->second;
            if (serviceNode != nullptr && serviceNode->IsActive())
            {
                return serviceNode;
            }
        }
        return nullptr;
    }

    ServiceNode *ServiceNodeComponent::GetNodeByNodeName(const std::string &nodeName)
    {
        for (ServiceNode *serviceNode : this->mServiceNodeArray)
        {
            if (serviceNode->IsActive() && serviceNode->GetNodeName() == nodeName)
            {
                return serviceNode;
            }
        }
		

        return nullptr;
    }

	ServiceNode *ServiceNodeComponent::GetNodeByServiceName(const std::string &service)
	{
		for (ServiceNode *serviceNode : this->mServiceNodeArray)
		{
			if (serviceNode->IsActive() && serviceNode->HasService(service))
			{
				return serviceNode;
			}
		}

		return nullptr;
	}

}// namespace Sentry
