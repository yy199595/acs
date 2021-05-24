#include"ServiceRegistry.h"
#include<Core/Applocation.h>
#include<Core/TcpSessionListener.h>
#include<Util/StringHelper.h>
#include<Manager/NetWorkManager.h>
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
		REGISTER_FUNCTION_2(ServiceRegistry::Register, ServiceDataList, ServiceDataList);
		REGISTER_FUNCTION_2(ServiceRegistry::QueryService, Int32Data, ServiceDataList);
		return true;
	}

	void ServiceRegistry::OnInitComplete()
	{
		
	}

	XCode ServiceRegistry::Register(long long operId, shared_ptr<ServiceDataList> actions, shared_ptr<ServiceDataList> returnData)
	{
		for (int index = 0; index < actions->services_size(); index++)
		{
			const ServiceData & serviceData = actions->services(index);
			int areaId = serviceData.adreid();
			const std::string & name = serviceData.service_name();
			const std::string & address = serviceData.service_address();
			const int serviceid = areaId << 32 | (this->mServiceIndex++);			
			
			ServiceData * retServiceData = returnData->add_services();
			if (retServiceData != nullptr)
			{
				retServiceData->CopyFrom(serviceData);
				retServiceData->set_service_id(serviceid);
			}

			SayNoDebugInfo("register service " << name << " " << address << " " << serviceid);
			this->mActionRegisterList.push_back(new ProxyService(name, address, serviceid, areaId));
		}
		return XCode::Successful;
	}

	XCode ServiceRegistry::QueryService(long long operId, shared_ptr<Int32Data> requestData, shared_ptr<ServiceDataList> returnData)
	{
		const int areaId = requestData->data();
		for (size_t index = 0; index < this->mActionRegisterList.size(); index++)
		{
			ProxyService * proxyService = this->mActionRegisterList[index];
			if (proxyService->GetAreaId() == 0 || proxyService->GetAreaId() == areaId)
			{
				ServiceData * serviceData = returnData->add_services();
				if (serviceData != nullptr)
				{
					serviceData->set_adreid(proxyService->GetAreaId());
					serviceData->set_service_id(proxyService->GetServiceId());
					serviceData->set_service_address(proxyService->GetAddress());
					serviceData->set_service_name(proxyService->GetServiceName());
				}
			}
		}
		return XCode::Successful;
	}

	void ServiceRegistry::AddActionInfo(ActionProxyInfo & actionInfo)
	{
		
	}
}
