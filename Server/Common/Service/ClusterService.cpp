#include"ClusterService.h"
#include<Manager/ServiceManager.h>
#include<NetWork/ActionScheduler.h>
#include<Coroutine/CoroutineManager.h>
#include<Service/ProxyService.h>
namespace SoEasy
{
	bool ClusterService::OnInit()
	{
		this->mRegistryService = nullptr;
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("AreaId", this->mAreaId));
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("NodeId", this->mNodeId));
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("QueryAddress", mQueryAddress));
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("ListenAddress", mListenAddress));
		SayNoAssertRetFalse_F(this->mServiceManager = this->GetManager<ServiceManager>());
		
		REGISTER_FUNCTION_1(ClusterService::RemoveService, Int32Data);
		REGISTER_FUNCTION_1(ClusterService::RefreshServices, ServicesNotice);
		return LocalService::OnInit();
	}

	void ClusterService::OnInitComplete()
	{
		this->mRegistryService = this->mServiceManager->AddProxyService(0, 0, "ServiceRegistry", this->mQueryAddress);
		if (this->mRegistryService != nullptr)
		{
			this->StarRegister();
			SayNoDebugInfo("start register local service to " << this->mQueryAddress);
		}
	}

	void ClusterService::OnConnectDone(SharedTcpSession tcpSession)
	{
		
		
	}

	void ClusterService::StarRegister()
	{
		std::vector<ServiceBase *> localServices;
		this->mServiceManager->GetLocalServices(localServices);
		
		shared_ptr<Service_NodeRegisterRequest> registerNode = make_shared<Service_NodeRegisterRequest>();

		registerNode->set_area_id(this->mAreaId);
		registerNode->set_node_id(this->mNodeId);
		registerNode->set_node_address(this->mListenAddress);

		XCode regNodeCode = this->mRegistryService->Call("RegisterNode", registerNode);

		if (regNodeCode != XCode::Successful)
		{
			SayNoDebugError("register node fail");
			return;
		}

		long long globalId = (long long)this->mAreaId << 32 | this->mNodeId;
		shared_ptr<Service_RegisterRequest> registerService = make_shared<Service_RegisterRequest>();
		
		int index = 0;
		registerService->set_id(globalId);
		for (ServiceBase * localService : localServices)
		{
			auto element = registerService->mutable_mservicemap();
			element->insert({ localService->GetServiceName(),  localService->GetServiceId() });
		}

		XCode regServiceCode = this->mRegistryService->Call("RegisterService", registerService);

		if (regServiceCode != XCode::Successful)
		{
			SayNoDebugError("register service fail");
			return;
		}
		SayNoDebugLog("register local service successful");
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
			const int areaId = serviceData.adreid();
			const int serviceId = serviceData.service_id();
			const std::string & address = serviceData.service_address();
			const std::string & serviceName = serviceData.service_name();
			this->mServiceManager->AddProxyService(areaId, serviceId, serviceName, address);
		}
		return XCode::Successful;
	}
}