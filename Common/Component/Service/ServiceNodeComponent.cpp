#include "ServiceNodeComponent.h"

#include <Core/App.h>
#include <Service/ServiceNode.h>
#include <Scene/ProtocolComponent.h>
#include <Network/Tcp/TcpNetSessionComponent.h>

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

	ServiceNode * ServiceNodeComponent::CreateNode(int areaId, int nodeId, std::string name, std::string address)
	{
		ServiceNode * serviceNode = new ServiceNode(areaId, nodeId, name, address);
		if (serviceNode != nullptr)
		{
			serviceNode->Init(name);
			this->mServiceNodeArray.push_back(serviceNode);
			this->mServiceNodeMap2.emplace(address, serviceNode);
            this->mServiceNodeMap1.emplace(serviceNode->GetNodeUId(), serviceNode);
            SayNoDebugInfo("create new service " << name << "  [" << address << "]");
		}
		return serviceNode;
	}

    bool ServiceNodeComponent::Awake()
    {
		ServerConfig & ServerCfg = App::Get().GetConfig();
		SayNoAssertRetFalse_F(ServerCfg.GetValue("AreaId", this->mAreaId));
		SayNoAssertRetFalse_F(ServerCfg.GetValue("CenterAddress", "ip", this->mCenterIp));
		SayNoAssertRetFalse_F(ServerCfg.GetValue("CenterAddress", "port", this->mCenterPort));
		SayNoAssertRetFalse_F(mProtocolComponent = this->GetComponent<ProtocolComponent>());
        const std::string centerAddress = this->mCenterIp + ":" + std::to_string(this->mCenterPort);
		return this->CreateNode(0, 0, "Center", centerAddress)->AddService("CenterService");
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
        for (ServiceNode *serviceNode: this->mServiceNodeArray)
        {
            if (serviceNode->IsActive() && serviceNode->HasService(service))
            {
                return serviceNode;
            }
        }

        s2s::NodeQuery_Request request;
        s2s::NodeQuery_Response response;
        request.set_areaid(this->mAreaId);
        request.set_servicename(service);
        ServiceNode *centerNode = this->GetServiceNode(0);
        XCode code = centerNode->Call("CenterService", "Query", request, response);

        ServiceNode * newServiceNode = nullptr;
        if (code == XCode::Successful && response.nodeinfos_size() > 0)
        {
            for (int index = 0; index < response.nodeinfos_size(); index++)
            {
                const int areaId = response.nodeinfos(index).uid() / 10000;
                const int nodeId = response.nodeinfos(index).uid() % 10000;
                const std::string &address = response.nodeinfos(index).address();
                const std::string &nodeName = response.nodeinfos(index).servername();
                newServiceNode = this->CreateNode(areaId, nodeId, nodeName, address);

            }
        }
        return newServiceNode;
    }

}// namespace Sentry
