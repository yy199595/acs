#include "ServiceManager.h"
#include <Service/LocalService.h>
#include <Service/LocalLuaService.h>

#include <Util/StringHelper.h>
#include <Core/ObjectRegistry.h>
#include<Manager/NetProxyManager.h>
#include<Manager/ActionManager.h>
#include <Coroutine/CoroutineManager.h>
namespace SoEasy
{

	bool ServiceManager::OnInit()
	{
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("NodeId", this->mNodeId));
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("Service", this->mServiceList));

		SayNoAssertRetFalse_F(this->mNetManager = this->GetManager<NetProxyManager>());
		SayNoAssertRetFalse_F(this->mActionManager = this->GetManager<ActionManager>());
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

	bool ServiceManager::HandlerMessage(PB::NetWorkPacket * messageData)
	{
		ServiceBase * localService = this->GetService(messageData->service());
		if (localService == nullptr || localService->HasMethod(messageData->method()))
		{
			return false;
		}
		this->mCorManager->Start(messageData->method(), [localService, this, messageData]()
			{
				long long rpcId = messageData->rpcid();
				XCode code = localService->InvokeMethod(messageData);
				if (rpcId != 0 && code != XCode::NotResponseMessage)
				{
					messageData->clear_method();
					messageData->clear_service();
					messageData->set_code((int)code);
					this->mActionManager->InvokeCallback(rpcId, messageData);
					NetPacketPool.Destory(messageData);
				}
			});
		return true;
	}

	bool ServiceManager::HandlerMessage(const std::string & address, PB::NetWorkPacket * messageData)
	{
		const std::string & method = messageData->method();
		const std::string & service = messageData->service();
		ServiceBase * localService = this->GetService(service);
		if (localService == nullptr || !localService->HasMethod(method))
		{
			SayNoDebugError("call function not find [" << service << "." << method << "]");
			return false;
		}

		this->mCorManager->Start(method, [address, localService, this, messageData]()
			{
				XCode code = localService->InvokeMethod(address, messageData);
				if (messageData->rpcid() == 0 || code == XCode::NotResponseMessage)
				{
					NetPacketPool.Destory(messageData);
				}
				else
				{
					messageData->clear_method();
					messageData->clear_service();
					messageData->set_code((int)code);
					this->mNetManager->SendMsgByAddress(address, messageData);
				}				
			});
		return true;
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
				this->mLuaServiceMap.emplace(name, service);
				SayNoDebugInfo("add new lua service " << name);
				return service;
			}
			SayNoDebugError("init lua service " << name << " fail");
		}
		return nullptr;
	}

	ServiceBase * ServiceManager::GetService(const std::string & name)
	{
		LocalLuaService * luaService = this->GetLuaService(name);
		if (luaService == nullptr)
		{
			return this->GetLocalService(name);
		}
		return luaService;
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
			this->mLocalServiceMap.emplace(name, localService);
		}
		return true;
	}

}
