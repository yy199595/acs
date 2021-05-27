#include "ServiceNode.h"

namespace SoEasy
{
	ServiceNode::ServiceNode(int areaId, int nodeId, const std::string & address, const std::string & naddress)
		: mAreaId(areaId), mNodeId(nodeId), mAddress(address), mNoticeAddress(naddress)
	{

	}

	ProxyService * ServiceNode::GetService(const std::string & name)
	{
		auto iter = this->mServiceMap.find(name);
		return iter != this->mServiceMap.end() ? iter->second : nullptr;
	}

	ProxyService * ServiceNode::CreateProxyService(const int serviceId, const std::string & name)
	{
		auto iter = this->mServiceMap.find(name);
		if (iter == this->mServiceMap.end())
		{
			ProxyService * proxyService = new ProxyService(this->mAddress, this->mAreaId);
			if (proxyService != nullptr)
			{
				Applocation * app = Applocation::Get();
				proxyService->InitService(name, serviceId);
				SayNoAssertRetFalse_F(proxyService->OnInit());
				this->mServiceMap.emplace(name, proxyService);
				return proxyService;
			}
			return nullptr;
		}
		return iter->second;
	}
	void ServiceNode::GetServices(std::vector<ProxyService*>& services)
	{
		auto iter = this->mServiceMap.begin();
		for (; iter != this->mServiceMap.end(); iter++)
		{
			services.push_back(iter->second);
		}
	}
}