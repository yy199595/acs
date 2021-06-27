#include"ServiceNodeManager.h"
#include<Other/ServiceNode.h>
namespace SoEasy
{
	bool ServiceNodeManager::DelServiceNode(int nodeId)
	{
		auto iter = this->mServiceNodeMap1.find(nodeId);
		if (iter != this->mServiceNodeMap1.end())
		{
			ServiceNode * serviceNode = iter->second;
			serviceNode->SetActive(false);
			const std::string & address = serviceNode->GetAddress();
			auto iter1 = this->mServiceNodeMap2.find(address);
			if (iter1 != this->mServiceNodeMap2.end())
			{
				this->mServiceNodeMap2.erase(iter1);
			}
			return true;
		}
		return false;
	}

	bool ServiceNodeManager::AddServiceNode(ServiceNode * serviceNode)
	{
		const int id = serviceNode->GetNodeId();
		const std::string & address = serviceNode->GetAddress();
		auto iter = this->mServiceNodeMap1.find(id);
		if (iter == this->mServiceNodeMap1.end())
		{
			SayNoDebugLog(serviceNode->GetJsonString());
			this->mServiceNodeMap1.emplace(id, serviceNode);
			this->mServiceNodeMap2.emplace(address, serviceNode);
			serviceNode->Init(this->GetApp(), serviceNode->GetNodeName());
			return true;
		}
		return false;
	}

	bool ServiceNodeManager::OnInit()
	{
		std::string queryAddress;
		this->GetConfig().GetValue("QueryAddress", queryAddress);
		ServiceNode * centerNode = new ServiceNode(0, 0, "Center", queryAddress);
		centerNode->AddService(std::string("ServiceRegistry"));
		return this->AddServiceNode(centerNode);
	}

	void ServiceNodeManager::OnSystemUpdate()
	{
		auto iter = this->mServiceNodeMap1.begin();
		for (; iter != this->mServiceNodeMap1.end();)
		{
			ServiceNode * serviceNode = iter->second;
			if (!serviceNode->IsActive())
			{
				delete serviceNode;
				this->mServiceNodeMap1.erase(iter++);
				continue;
			}
			iter++;
			serviceNode->OnSystemUpdate();
		}
	}

	ServiceNode * ServiceNodeManager::GetServiceNode(const int nodeId)
	{
		auto iter = this->mServiceNodeMap1.find(nodeId);
		return iter != this->mServiceNodeMap1.end() ? iter->second : nullptr;
	}
	ServiceNode * ServiceNodeManager::GetServiceNode(const std::string & address)
	{
		auto iter = this->mServiceNodeMap2.find(address);
		return iter != this->mServiceNodeMap2.end() ? iter->second : nullptr;
	}
}
