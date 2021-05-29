#include"ServiceManager.h"
#include<Service/LocalService.h>
#include<Service/ProxyService.h>
#include<Service/LocalLuaService.h>

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
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("NodeId", this->mNodeId));
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("Service", this->mServiceList));
		
		SayNoAssertRetFalse_F(this->mCorManager = this->GetManager<CoroutineManager>());
		return this->CreateLocalService();
	}

	void ServiceManager::OnInitComplete()
	{
		auto iter = this->mLocalServiceMap.begin();
		for (; iter != this->mLocalServiceMap.end(); iter++)
		{
			LocalService * localService = iter->second;
			auto cb = BIND_ACTION_0(LocalService::OnInitComplete, localService);
			this->mCorManager->Start(localService->GetServiceName() + ".Init", cb);
		}
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
		auto iter1 = this->mLocalServiceMap.begin();
		for (; iter1 != this->mLocalServiceMap.end(); iter1++)
		{
			LocalService * localService = iter1->second;
			localService->OnConnectDone(tcpSession);
		}
		auto iter2 = this->mProxyServiceMap.begin();
		for (; iter2 != this->mProxyServiceMap.end(); iter2++)
		{
			ProxyService * proxyService = iter2->second;
			proxyService->OnConnectDone(tcpSession);
		}
	}

	LocalLuaService * ServiceManager::AddLuaService(const std::string name, LocalLuaService * service)
	{
		auto iter = this->mLuaServiceMap.find(name);
		if (iter == this->mLuaServiceMap.end())
		{			
			int serviceId = 0;
			LocalService * localService = this->GetLocalService(name);
			if (localService != nullptr)
			{
				serviceId = localService->GetServiceId();
			}
			else
			{
				size_t index = this->mServiceList.size() + 1;
				serviceId = (int)this->mNodeId << 16 | index;			
			}

			service->InitService(name, serviceId);
			if (service->OnInit())
			{
				this->mServiceList.push_back(name);
				this->mReadyServiceQueue.push(service);
				this->mLuaServiceMap.emplace(name, service);
				SayNoDebugInfo("add new lua service " << name << " " << serviceId);
				return service;
			}
			SayNoDebugError("init lua service " << name << " fail");
		}
		return nullptr;
	}

	ProxyService * ServiceManager::AddProxyService(int areaId, int serviceId, const std::string name, const std::string address)
	{
		auto iter = this->mProxyServiceMap.find(serviceId);
		if (iter == this->mProxyServiceMap.end())
		{
			ProxyService * proxyService = new ProxyService(address, areaId);
			if (proxyService != nullptr)
			{
				proxyService->InitService(name, serviceId);
				if (proxyService->OnInit())
				{
					this->mProxyServiceMap.emplace(serviceId, proxyService);
					SayNoDebugInfo("add new proxy service " << name << " " << serviceId);
					return proxyService;
				}
			}
			return nullptr;
		}
		return iter->second;
	}

	LocalService * ServiceManager::GetLocalService(const std::string & name)
	{
		auto iter = this->mLocalServiceMap.find(name);
		return iter != this->mLocalServiceMap.end() ? iter->second : nullptr;
	}

	LocalLuaService * ServiceManager::GetLuaService(const std::string & name)
	{
		auto iter = this->mLuaServiceMap.find(name);
		return iter != this->mLuaServiceMap.end() ? iter->second : nullptr;
	}

	SharedTcpSession ServiceManager::GetProxySession(const std::string & address)
	{
		auto iter = this->mActionSessionMap.find(address);
		if (iter == this->mActionSessionMap.end())
		{
			std::string ip;
			unsigned short port;
			StringHelper::ParseIpAddress(address, ip, port);
			SharedTcpSession tcpSession = make_shared<TcpClientSession>(this, "ServiceSession", ip, port);
			if (tcpSession->StartConnect())
			{
				this->mActionSessionMap.emplace(address, tcpSession);
				return tcpSession;
			}
			return nullptr;
		}
		return iter->second;
	}

	bool ServiceManager::RemoveProxyervice(const int id)
	{
		auto iter = this->mProxyServiceMap.find(id);
		if (iter != this->mProxyServiceMap.end())
		{
			iter->second->SetActive(false);
			return true;
		}
		return false;
	}

	void ServiceManager::GetLocalServices(std::vector<ServiceBase*> & services)
	{
		services.clear();
		auto iter = this->mLocalServiceMap.begin();
		for (; iter != this->mLocalServiceMap.end(); iter++)
		{
			services.push_back(iter->second);
		}
		auto iter1 = this->mLuaServiceMap.begin();
		for (; iter1 != this->mLuaServiceMap.end(); iter1++)
		{
			services.push_back(iter1->second);
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

	bool ServiceManager::CreateLocalService()
	{
		Applocation * app = Applocation::Get();
		for (size_t index = 0; index < this->mServiceList.size(); index++)
		{
			const std::string & name = this->mServiceList[index];
			LocalService * localService = ObjectRegistry<LocalService>::Create(name);
			if (localService == nullptr)
			{
				SayNoDebugError("create " << name << " service fail");
				return false;
			}
			int serviceId = (int)(this->mNodeId) << 16 | (index);
			localService->InitService(name, serviceId);
			if (!localService->OnInit())
			{
				SayNoDebugError("init " << name << " service fail");
				return false;
			}
			SayNoDebugLog("add new service " << name << " id " << serviceId);
			this->mLocalServiceMap.emplace(name, localService);
		}
		return true;
	}

}
