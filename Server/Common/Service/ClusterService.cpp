#include"ClusterService.h"
#include<Manager/ServiceManager.h>
#include<NetWork/ActionScheduler.h>
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{
	bool ClusterService::OnInit()
	{
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("AreaId", this->mAreaId));
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("QueryAddress", mQueryAddress));
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("ListenAddress", mListenAddress));
		SayNoAssertRetFalse_F(this->mServiceManager = this->GetManager<ServiceManager>());
		
		REGISTER_FUNCTION_1(ClusterService::RemoveService, Int32Data);
		REGISTER_FUNCTION_1(ClusterService::RefreshServices, ServicesNotice);
		return LocalService::OnInit();
	}

	void ClusterService::OnInitComplete()
	{
		this->mQuerySession = this->mServiceManager->GetProxySession(this->mQueryAddress);
	}

	void ClusterService::OnConnectDone(SharedTcpSession tcpSession)
	{
		if (this->mQuerySession != tcpSession)
		{
			return;
		}
		SayNoDebugInfo("start register local service to " << this->mQueryAddress);
		this->Start("ServiceRegister", BIND_THIS_ACTION_0(ClusterService::StarRegister));
	}

	void ClusterService::StarRegister()
	{
		std::vector<LocalService *> localServices;
		this->mServiceManager->GetLocalServices(localServices);
		shared_ptr<ServiceRegister_Request> registerService = make_shared<ServiceRegister_Request>();

		registerService->set_area_id(this->mAreaId);
		registerService->set_service_address(this->mListenAddress);
		for (LocalService * localService : localServices)
		{
			std::string * serviceName = registerService->add_service_names();
			serviceName->assign(localService->GetServiceName());
		}
		PB::ServiceRegister_Respond retServiceDatas;
		ActionScheduler actionSchduler(this->mQuerySession);
		actionSchduler.Call("ServiceRegistry", "Register", registerService, retServiceDatas);

		auto iter = retServiceDatas.service_infos().begin();
		for (; iter != retServiceDatas.service_infos().end(); iter++)
		{
			const std::string serviceName = iter->first;
			LocalService * localService = this->mServiceManager->GetLocalService(serviceName);
			if (localService != nullptr)
			{
				localService->InitService(iter->second);
				SayNoDebugInfo("register " << serviceName << " successful " << iter->second);
			}
		}
	}

	XCode ClusterService::RemoveService(long long, shared_ptr<Int32Data> serviceData)
	{
		const int serviceId = serviceData->data();
		
		return XCode::Successful;
	}

	XCode ClusterService::RefreshServices(long long, shared_ptr<ServicesNotice> retServiceDatas)
	{
		for (int index = 0; index < retServiceDatas->services_size(); index++)
		{
			const ServiceData & serviceData = retServiceDatas->services(index);
			const int mAreaId = serviceData.adreid();
			const int serviceId = serviceData.service_id();
			const std::string & address = serviceData.service_address();
			const std::string & serviceName = serviceData.service_name();
			LocalService * localService = this->mServiceManager->GetLocalService(serviceData.service_name());
			if (localService != nullptr)
			{
				localService->InitService(serviceId);
				SayNoDebugInfo("register " << serviceData.service_name() << " success");
			}
			this->mServiceManager->AddProxyService(serviceName, mAreaId, serviceId, address);
		}
		return XCode::Successful;
	}
}