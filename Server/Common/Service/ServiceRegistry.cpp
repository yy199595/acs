#include"ServiceRegistry.h"
#include<Core/Applocation.h>
#include<Core/TcpSessionListener.h>
#include<Util/StringHelper.h>
#include<Manager/NetWorkManager.h>
#include<NetWork/ActionScheduler.h>
#include<NetWork/RemoteScheduler.h>
#include<Protocol/ServerCommon.pb.h>
#include<Coroutine/CoroutineManager.h>

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
		REGISTER_FUNCTION_2(ServiceRegistry::Register, ServiceRegister_Request, ServiceRegister_Respond);
		return true;
	}

	void ServiceRegistry::OnInitComplete()
	{

	}

	XCode ServiceRegistry::Register(long long operId, shared_ptr<ServiceRegister_Request> actionInfo, shared_ptr<ServiceRegister_Respond> returnData)
	{
		int areaId = actionInfo->area_id();
		const std::string & address = actionInfo->service_address();
		for (int index = 0; index < actionInfo->service_names_size(); index++)
		{
			const std::string & name = actionInfo->service_names(index);
			const int serviceId = this->AddService(areaId, name, address);
			returnData->mutable_service_infos()->insert({ name, serviceId });
			SayNoDebugLog("register service " << areaId << " " << name << "  " << address);
		}
		SharedTcpSession tcpSession = this->GetCurTcpSession();
		auto iter = this->mQuerySessionMap.find(areaId);
		if (iter == this->mQuerySessionMap.end())
		{
			std::set<std::string> tempArray;
			this->mQuerySessionMap.emplace(areaId, tempArray);
		}
		this->mQuerySessionMap[areaId].insert(tcpSession->GetAddress());

		this->RefreshServices(areaId);
		return XCode::Successful;
	}

	void ServiceRegistry::RefreshServices(int areaId)
	{
		ServicesNotice serviceDatas;
		for (auto iter = this->mServiceMap.begin(); iter != this->mServiceMap.end(); iter++)
		{
			ProxyService * service = iter->second;
			if (service != nullptr && (service->GetAreaId() == areaId || service->GetAreaId() == 0))
			{
				ServiceData * serviceData = serviceDatas.add_services();
				if (serviceData != nullptr)
				{
					serviceData->set_adreid(service->GetAreaId());
					serviceData->set_service_id(service->GetServiceId());
					serviceData->set_service_address(service->GetAddress());
					serviceData->set_service_name(service->GetServiceName());
				}
			}
		}

		auto iter1 = this->mQuerySessionMap.find(areaId);
		if (iter1 != this->mQuerySessionMap.end())
		{
			for (const std::string & address : iter1->second)
			{
				SharedTcpSession tcpSession = this->mNetWorkManager->GetTcpSession(address);
				if (tcpSession != nullptr)
				{
					RemoteScheduler remoteShceduler(tcpSession);
					remoteShceduler.Call("ClusterService", "RefreshServices", &serviceDatas);
				}
			}
		}
	}

	int ServiceRegistry::AddService(int areaId, const std::string & name, const std::string & address)
	{
		auto iter = this->mServiceMap.begin();
		for (; iter != this->mServiceMap.end(); iter++)
		{
			ProxyService * service = iter->second;
			if (service->GetAreaId() == areaId && service->GetServiceName() == name && service->GetAddress() == address)
			{
				return service->GetServiceId();
			}
		}
		const int serviceId = this->mServiceIndex++;
		ProxyService * proxyService = new ProxyService(name, address, serviceId, areaId);
		if (proxyService != nullptr)
		{
			proxyService->Init(this->GetApp(), name);
			this->mServiceMap.insert(std::make_pair(serviceId, proxyService));
			return proxyService->GetServiceId();
		}
		return 0;
	}
}
