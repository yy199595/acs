#include"ServiceManager.h"
#include<Service/LocalService.h>
#include<Service/ProxyService.h>

#include<Util/StringHelper.h>
#include<Core/ObjectRegistry.h>

#include<Manager/ListenerManager.h>
#include<Coroutine/CoroutineManager.h>

#include<NetWork/ActionScheduler.h>
namespace SoEasy
{
	
	bool ServiceManager::OnInit()
	{
		SayNoAssertRetFalse_F(SessionManager::OnInit());
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("AreaId", this->mAreaId));
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("Service", this->mServiceList));
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("QueryAddress", mQueryAddress));
		SayNoAssertRetFalse_F(StringHelper::ParseIpAddress(mQueryAddress, mQueryIp, mQueryPort));

		SayNoAssertRetFalse_F(this->mListenerManager = this->GetManager<ListenerManager>());
		SayNoAssertRetFalse_F(this->mCoroutineManager = this->GetManager<CoroutineManager>());

		this->mListenAddress = this->mListenerManager->GetAddress();
		this->mQuerySession = make_shared<TcpClientSession>(this, "QuerySession", mQueryIp, mQueryPort);
		return this->CreateLocalService();
	}

	void ServiceManager::OnInitComplete()
	{
		this->mQuerySession->StartConnect();
		CoroutineManager * coroutineMgr = this->GetManager<CoroutineManager>();
		auto iter = this->mLocalServiceMap.begin();
		for (; iter != this->mLocalServiceMap.end(); iter++)
		{
			LocalService * localService = iter->second;
			SayNoDebugInfo("start " << iter->first << " service successful");
			coroutineMgr->Start(localService->GetServiceName(), std::bind(&LocalService::OnInitComplete, localService));
		}
	}

	void ServiceManager::OnSystemUpdate()
	{
		SessionManager::OnSystemUpdate();
		auto iter = this->mLocalServiceMap.begin();
		for (; iter != this->mLocalServiceMap.end(); iter++)
		{
			LocalService * localService = iter->second;
			if (localService != nullptr && localService->IsActive())
			{
				localService->OnSystemUpdate();
			}		
		}
		auto iter1 = this->mProxyServiceMap.begin();
		for (; iter1 != this->mProxyServiceMap.end(); iter1++)
		{
			ProxyService * proxyService = iter1->second;
			if (proxyService != nullptr && proxyService->IsActive())
			{
				proxyService->OnSystemUpdate();
			}		
		}
	}

	void ServiceManager::OnSessionErrorAfter(SharedTcpSession tcpSession)
	{

	}

	void ServiceManager::OnSessionConnectAfter(SharedTcpSession tcpSession)
	{
		if (tcpSession == this->mQuerySession)
		{
			SayNoDebugInfo("connect query session successful");
			this->mCoroutineManager->Start("ServiceOper", BIND_THIS_ACTION_0(ServiceManager::StartRegister));		
		}
	}

	bool ServiceManager::NewLocalService(const std::string & name)
	{
		LocalService * service = ObjectRegistry<LocalService>::Create(name);
		if (service == nullptr)
		{
			SayNoDebugError("create " << name << " service fail");
			return false;
		}
		
		if (!service->Init(this->GetApp(), name) || !service->OnInit())
		{
			SayNoDebugError("init " << name << " service fail");
			return false;
		}
		SayNoDebugLog("add new service " << name);
		const std::string newName = name + ".Init";
		this->mLocalServiceMap.emplace(name, service);
		this->mCoroutineManager->Start(newName, BIND_ACTION_0(LocalService::OnInitComplete, service));		
		return true;
	}

	LocalService * ServiceManager::GetLocalService(const std::string & name)
	{
		auto iter = this->mLocalServiceMap.find(name);
		return iter != this->mLocalServiceMap.end() ? iter->second : nullptr;
	}

	SharedTcpSession ServiceManager::GetProxySession(const std::string & address)
	{
		auto iter = this->mActionSessionMap.find(address);
		if (iter == this->mActionSessionMap.end())
		{
			std::string ip;
			unsigned short port;
			StringHelper::ParseIpAddress(address, ip, port);
			SharedTcpSession tcpSession = make_shared<TcpClientSession>(this, "ProxySession", ip, port);
			if (tcpSession->StartConnect())
			{
				this->mActionSessionMap.emplace(address, tcpSession);
				return tcpSession;
			}
			return nullptr;
		}
		return iter->second;
	}

	void ServiceManager::GetLocalServices(std::vector<LocalService*> services)
	{
		services.clear();
		auto iter = this->mLocalServiceMap.begin();
		for (; iter != this->mLocalServiceMap.end(); iter++)
		{
			services.push_back(iter->second);
		}
	}

	ProxyService * ServiceManager::GetProxyService(int id)
	{
		auto iter = this->mProxyServiceMap.find(id);
		return iter != this->mProxyServiceMap.end() ? iter->second : nullptr;
	}

	ProxyService * ServiceManager::GetProxyService(const std::string & name)
	{
		return nullptr;
	}

	void ServiceManager::StartRegister()
	{
		shared_ptr<ServiceDataList> serviceList = make_shared<ServiceDataList>();
		auto iter = this->mLocalServiceMap.begin();
		for (; iter != this->mLocalServiceMap.end(); iter++)
		{
			const std::string & name = iter->first;
			ServiceData * serviceData = serviceList->add_services();
			if (serviceData != nullptr)
			{
				serviceData->set_adreid(mAreaId);
				serviceData->set_service_name(name);
				serviceData->set_service_address(mListenAddress);
			}
		}
		ServiceDataList retServiceDatas;
		ActionScheduler actionSchduler(this->mQuerySession);
		actionSchduler.Call("ServiceRegistry", "Register", serviceList, retServiceDatas);
		for (int index = 0; index < retServiceDatas.services_size(); index++)
		{
			const ServiceData & serviceData = retServiceDatas.services(index);
			LocalService * localService = this->GetLocalService(serviceData.service_name());
			if (localService != nullptr)
			{
				localService->InitService(serviceData.service_name(), serviceData.service_id());
				SayNoDebugLog(serviceData.service_name() << " register sucessful " << serviceData.service_id());
			}
		}

		retServiceDatas.Clear();
		shared_ptr<Int32Data> data = make_shared<Int32Data>();
		data->set_data(this->mAreaId);
		actionSchduler.Call("ServiceRegistry", "QueryService", data, retServiceDatas);
		for (int index = 0; index < retServiceDatas.services_size(); index++)
		{
			const ServiceData & serviceData = retServiceDatas.services(index);
			const int serviceId = serviceData.service_id();

			auto iter = this->mProxyServiceMap.find(serviceId);
			if (iter == this->mProxyServiceMap.end())
			{
				const int areaId = serviceData.adreid();
				const std::string & address = serviceData.service_address();
				const std::string & serviceName = serviceData.service_name();
				this->mProxyServiceMap.emplace(serviceId, new ProxyService(serviceName, address, serviceId, areaId));
			}		
		}
	}

	bool ServiceManager::CreateLocalService()
	{
		Applocation * app = Applocation::Get();
		for (size_t index = 0; index < this->mServiceList.size(); index++)
		{
			const std::string & name = this->mServiceList[index];
			if (this->NewLocalService(name) == false)
			{
				return false;
			}
		}
		return true;
	}

}
