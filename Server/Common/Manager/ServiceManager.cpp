#include "ServiceManager.h"
#include <Service/LocalService.h>
#include <Service/ProxyService.h>
#include <Service/LocalLuaService.h>

#include <Util/StringHelper.h>
#include <Core/ObjectRegistry.h>

#include <Manager/ListenerManager.h>
#include <Coroutine/CoroutineManager.h>

#include <NetWork/ActionScheduler.h>
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
			LocalService *localService = iter->second;
			auto cb = BIND_ACTION_0(LocalService::OnInitComplete, localService);
			this->mCorManager->Start(localService->GetServiceName() + ".Init", cb);
		}
	}

	void ServiceManager::OnSystemUpdate()
	{
		SessionManager::OnSystemUpdate();
		for (size_t index = 0; index < this->mServiceVector.size(); index++)
		{
			ServiceBase *service = this->mServiceVector[index];
			if (service != nullptr && service->IsActive())
			{
				service->OnSystemUpdate();
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
			LocalService *localService = iter1->second;
			localService->OnConnectDone(tcpSession);
		}
	}

	LocalLuaService *ServiceManager::AddLuaService(const std::string name, LocalLuaService *service)
	{
		auto iter = this->mLuaServiceMap.find(name);
		if (iter == this->mLuaServiceMap.end())
		{
			service->InitService(name);
			if (service->OnInit())
			{
				this->mServiceList.push_back(name);
				this->mServiceVector.push_back(service);
				this->mLuaServiceMap.emplace(name, service);
				SayNoDebugInfo("add new lua service " << name);
				return service;
			}
			SayNoDebugError("init lua service " << name << " fail");
		}
		return nullptr;
	}

	LocalService *ServiceManager::GetLocalService(const std::string &name)
	{
		auto iter = this->mLocalServiceMap.find(name);
		return iter != this->mLocalServiceMap.end() ? iter->second : nullptr;
	}

	LocalLuaService *ServiceManager::GetLuaService(const std::string &name)
	{
		auto iter = this->mLuaServiceMap.find(name);
		return iter != this->mLuaServiceMap.end() ? iter->second : nullptr;
	}

	void ServiceManager::GetLocalServices(std::vector<ServiceBase *> &services)
	{
		auto iter = this->mLocalServiceMap.begin();
		for (; iter != this->mLocalServiceMap.end(); iter++)
		{
			LocalService *service = iter->second;
			if (service != nullptr && service->IsActive())
			{
				services.push_back(service);
			}
		}
		auto iter1 = this->mLuaServiceMap.begin();
		for (; iter1 != this->mLuaServiceMap.end(); iter1++)
		{
			LocalLuaService *service = iter1->second;
			if (service != nullptr && service->IsActive())
			{
				services.push_back(service);
			}
		}
	}


	void ServiceManager::GetLocalServices(std::vector<std::string> &serviceNames)
	{
		serviceNames.clear();
		auto iter = this->mLocalServiceMap.begin();
		for (; iter != this->mLocalServiceMap.end(); iter++)
		{
			LocalService *service = iter->second;
			if (service != nullptr && service->IsActive())
			{
				serviceNames.push_back(service->GetServiceName());
			}
		}
		auto iter1 = this->mLuaServiceMap.begin();
		for (; iter1 != this->mLuaServiceMap.end(); iter1++)
		{
			LocalLuaService *service = iter1->second;
			if (service != nullptr && service->IsActive())
			{
				serviceNames.push_back(service->GetServiceName());
			}
		}
	}

	bool ServiceManager::CreateLocalService()
	{
		Applocation *app = Applocation::Get();
		for (size_t index = 0; index < this->mServiceList.size(); index++)
		{
			const std::string &name = this->mServiceList[index];
			LocalService *localService = ObjectRegistry<LocalService>::Create(name);
			if (localService == nullptr)
			{
				SayNoDebugError("create " << name << " service fail");
				return false;
			}
			localService->InitService(name);
			if (!localService->OnInit())
			{
				SayNoDebugError("init " << name << " service fail");
				return false;
			}
			SayNoDebugLog("add new service " << name);
			this->mServiceVector.push_back(localService);
			this->mLocalServiceMap.emplace(name, localService);
		}
		return true;
	}

}
