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

		SayNoAssertRetFalse_F(this->mCorManager = this->GetManager<CoroutineManager>());
		SayNoAssertRetFalse_F(this->mListenerManager = this->GetManager<ListenerManager>());
		

		this->mListenAddress = this->mListenerManager->GetAddress();
		this->mQuerySession = make_shared<TcpClientSession>(this, "QuerySession", mQueryIp, mQueryPort);
		return this->CreateLocalService();
	}

	void ServiceManager::OnInitComplete()
	{
		this->mQuerySession->StartConnect();
	}

	void ServiceManager::OnSystemUpdate()
	{
		SessionManager::OnSystemUpdate();
		auto iter = this->mLocalServiceMap.begin();
		for (; iter != this->mLocalServiceMap.end(); iter++)
		{
			LocalService * localService = iter->second;
			if (localService->IsActive())
			{
				localService->OnSystemUpdate();
			}
		}
		auto iter1 = this->mProxyServiceMap.begin();
		for (; iter1 != this->mProxyServiceMap.end(); iter1++)
		{
			ProxyService * proxyService = iter1->second;
			if (proxyService->IsActive())
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
			this->mCorManager->Start("ServiceOper", BIND_THIS_ACTION_0(ServiceManager::StartRegister));
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
		this->mLocalServiceMap.emplace(name, service);	
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
		//注册本机服务
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
	
		//查询远程服务
		this->QueryRemoteService(); 

		//初始化本机服务
		for (int index = 0; index < retServiceDatas.services_size(); index++)
		{
			const ServiceData & serviceData = retServiceDatas.services(index);
			LocalService * localService = this->GetLocalService(serviceData.service_name());
			if (localService != nullptr)
			{
				const std::string name = serviceData.service_name() + ".Init";
				localService->InitService(serviceData.service_name(), serviceData.service_id());
				this->mCorManager->Start(name, BIND_ACTION_0(LocalService::OnInitComplete, localService));
				SayNoDebugLog(serviceData.service_name() << " register sucessful " << serviceData.service_id());
			}
		}
	}

	void ServiceManager::QueryRemoteService()
	{
		shared_ptr<Int32Data> data = make_shared<Int32Data>();
		data->set_data(this->mAreaId);

		ServiceDataList retServiceDatas;
		ActionScheduler actionSchduler(this->mQuerySession);
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
				const std::string & serviceName = serviceData.service_name();//在本机创建远程服务副本
				ProxyService * proxyService = new ProxyService(serviceName, address, serviceId, areaId);
				if (proxyService != nullptr)
				{
					this->mProxyServiceMap.emplace(serviceId, proxyService);
					const std::string name = serviceData.service_name() + ".Init";
					this->mCorManager->Start(name, BIND_ACTION_0(ProxyService::OnInitComplete, proxyService));
				}
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
