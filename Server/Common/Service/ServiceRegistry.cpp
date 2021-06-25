#include"ServiceRegistry.h"
#include<Core/Applocation.h>
#include<Core/TcpSessionListener.h>
#include<Util/StringHelper.h>
#include<Manager/NetWorkManager.h>
#include<NetWork/ActionScheduler.h>
#include<NetWork/RemoteScheduler.h>
#include<Protocol/ServerCommon.pb.h>
#include<Coroutine/CoroutineManager.h>

#include<Other/ServiceNode.h>
#include<Service/ProxyService.h>

namespace SoEasy
{
	ServiceRegistry::ServiceRegistry()
	{

	}

	bool ServiceRegistry::OnInit()
	{
		this->mServiceIndex = 100;
		SayNoAssertRetFalse_F(LocalService::OnInit());
		SayNoAssertRetFalse_F(this->mNetWorkManager = this->GetManager<NetWorkManager>());
		REGISTER_FUNCTION_1(ServiceRegistry::RegisterNode, Service_NodeRegisterRequest);
		REGISTER_FUNCTION_1(ServiceRegistry::RegisterService, Service_RegisterRequest);

		return true;
	}

	void ServiceRegistry::OnSystemUpdate()
	{
		LocalService::OnSystemUpdate();
		for (size_t index = 0; index < this->mProxyServices.size(); index++)
		{
			ProxyService * proxyService = this->mProxyServices[index];
			proxyService->OnSystemUpdate();
		}
	}

	void ServiceRegistry::OnInitComplete()
	{

	}

	XCode ServiceRegistry::RegisterNode(long long id, shared_ptr<Service_NodeRegisterRequest> nodeInfo)
	{
		const int areaId = nodeInfo->area_id();
		const int nodeId = nodeInfo->node_id();
		const std::string address = nodeInfo->node_address();
		long long key = (long long)areaId << 32 | nodeId;
		auto iter = this->mServiceNodeMap.find(key);
		if (iter != this->mServiceNodeMap.end())
		{
			return XCode::Failure;
		}
		SharedTcpSession tcpSession = this->GetCurTcpSession();
		ServiceNode * node = new ServiceNode(areaId, nodeId, address, tcpSession->GetAddress());
		this->mServiceNodeMap.emplace(key, node);
		return XCode::Successful;
	}

	XCode ServiceRegistry::RegisterService(long long id, shared_ptr<Service_RegisterRequest> serviceInfos)
	{
		long long nodeId = serviceInfos->id();
		auto iter = this->mServiceNodeMap.find(nodeId);
		if (iter == this->mServiceNodeMap.end())
		{
			return XCode::Failure;
		}
		ServiceNode * serviceNode = iter->second;
		auto iter1 = serviceInfos->mservicemap().begin();
		for (; iter1 != serviceInfos->mservicemap().end(); iter1++)
		{
			const int serviceId = iter1->second;
			const std::string name = iter1->first;
			ProxyService * proxyService = serviceNode->CreateProxyService(serviceId, name);
			if (proxyService == nullptr)
			{
				return XCode::Failure;
			}
			this->mProxyServices.push_back(proxyService);
			SayNoDebugInfo("register service " << name << " " << serviceId);
		}
		int areaId = nodeId >> 32;	
		return this->RefreshServices(areaId);
	}

	XCode ServiceRegistry::RefreshServices(int areaId)
	{
		std::vector<ProxyService *> clusterServices;
		shared_ptr<ServicesNotice> serviceDatas = make_shared<ServicesNotice>();
		for (auto iter = this->mServiceNodeMap.begin(); iter != this->mServiceNodeMap.end(); iter++)
		{
			ServiceNode * serviceNode = iter->second;
			if (serviceNode->GetAreaId() == 0 || serviceNode->GetAreaId() == areaId)
			{
				std::vector<ProxyService *> proxyServices;
				serviceNode->GetServices(proxyServices);
				for (size_t index = 0; index < proxyServices.size(); index++)
				{
					ProxyService * service = proxyServices[index];
					ServiceData * serviceData = serviceDatas->add_services();
					serviceData->set_adreid(service->GetAreaId());
					serviceData->set_service_id(service->GetServiceId());
					serviceData->set_service_address(service->GetAddress());
					serviceData->set_service_name(service->GetServiceName());
				}
				ProxyService * clusterService = serviceNode->GetService("ClusterService");
				if (clusterService == nullptr)
				{
					return XCode::Failure;
				}
				clusterServices.push_back(clusterService);
			}
		}

		for (size_t index = 0; index < clusterServices.size(); index++)
		{
			ProxyService * clusterService = clusterServices[index];
			clusterService->Notice("RefreshServices", serviceDatas);
		}
		return XCode::Successful;
	}
}
