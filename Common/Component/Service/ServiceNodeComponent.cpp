#include "ServiceNodeComponent.h"

#include <Core/App.h>
#include <Service/ServiceNode.h>
#include <Scene/SceneSessionComponent.h>
#include <Scene/SceneProtocolComponent.h>
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

    bool ServiceNodeComponent::DelNode(const std::string &address)
    {
        auto iter1 = this->mServiceNodeMap2.find(address);
        if (iter1 != this->mServiceNodeMap2.end())
        {
            ServiceNode *serviceNode = iter1->second;
            if (serviceNode != nullptr)
            {
                serviceNode->SetActive(false);
                const int nodeId = serviceNode->GetNodeId();
                auto iter2 = this->mServiceNodeMap1.find(nodeId);
                if (iter2 != this->mServiceNodeMap1.end())
                {
                    this->mServiceNodeMap1.erase(iter2);
                }
            }
            this->mServiceNodeMap2.erase(iter1);
            return true;
        }
        return false;
    }

	ServiceNode * ServiceNodeComponent::CreateNode(int areaId, int nodeId, std::string name, std::string address)
	{
		auto iter = this->mServiceNodeMap1.find(nodeId);
		if (iter != this->mServiceNodeMap1.end())
		{
			return nullptr;
		}
		ServiceNode * serviceNode = new ServiceNode(areaId, nodeId, name, address);
		if (serviceNode != nullptr)
		{
			serviceNode->Init(name);
			this->mServiceNodeArray.push_back(serviceNode);
			this->mServiceNodeMap1.emplace(nodeId, serviceNode);
			this->mServiceNodeMap2.emplace(address, serviceNode);
			SayNoDebugInfo("create new service " << name << "  [" << address << "]");
		}
		return serviceNode;
	}

    bool ServiceNodeComponent::Awake()
    {
		ServerConfig & ServerCfg = App::Get().GetConfig();
		SayNoAssertRetFalse_F(ServerCfg.GetValue("CenterAddress", "ip", this->mCenterIp));
		SayNoAssertRetFalse_F(ServerCfg.GetValue("CenterAddress", "port", this->mCenterPort));
		SayNoAssertRetFalse_F(mProtocolComponent = Scene::GetComponent<SceneProtocolComponent>());
        const std::string centerAddress = this->mCenterIp + ":" + std::to_string(this->mCenterPort);       	
		return this->CreateNode(0, 0, "Center", centerAddress)->AddService("ServiceCenter");
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
