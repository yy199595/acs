#include "ServiceManager.h"
#include <Service/LocalService.h>
#include <Service/LocalLuaService.h>

#include <Util/StringHelper.h>
#include <Core/ObjectRegistry.h>
#include<Manager/NetWorkManager.h>
#include<Manager/ActionManager.h>
#include <Coroutine/CoroutineManager.h>
namespace SoEasy
{

	bool ServiceManager::OnInit()
	{
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("NodeId", this->mNodeId));
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("Service", this->mServiceList));

		SayNoAssertRetFalse_F(this->mNetManager = this->GetManager<NetWorkManager>());
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

	void ServiceManager::OnSystemUpdate()
	{
		SharedNetPacket messageBuffer;
		this->mRemoteMessageQueue.SwapQueueData();
		// 远程调用消息处理
		while (this->mRemoteMessageQueue.PopItem(messageBuffer))
		{
			const std::string & address = messageBuffer->mAddress;
			const SharedPacket & messageData = messageBuffer->mMessagePacket;
			ServiceBase * localService = this->GetService(messageData->service());
			if (localService != nullptr)
			{
				this->mCorManager->Start(messageData->method(), [address, localService, this, messageData]()
					{
						SharedPacket responseData = make_shared<NetWorkPacket>();															
						XCode code = localService->InvokeMethod(address, messageData, responseData);
						if (messageData->rpcid() != 0 && code != XCode::NotResponseMessage)
						{
							responseData->set_code(code);
							responseData->set_rpcid(messageData->rpcid());
							responseData->set_entityid(messageData->entityid());
							this->mNetManager->SendMessageByAdress(address, responseData);
						}
					});
			}
			// 本机调用消息处理
			while (!this->mLocalMessageQueue.empty())
			{
				const SharedPacket & messageData = this->mLocalMessageQueue.front();
				ServiceBase * localService = this->GetService(messageData->service());
				if (localService != nullptr)
				{
					this->mCorManager->Start(messageData->method(), [localService, this, messageData]()
						{
							SharedPacket responseData = make_shared<NetWorkPacket>();					
							XCode code = localService->InvokeMethod(messageData, responseData);
							if (messageData->rpcid() != 0 && code != XCode::NotResponseMessage)
							{
								responseData->set_code(code);
								responseData->set_rpcid(messageData->rpcid());
								responseData->set_entityid(messageData->entityid());
								this->mActionManager->PushLocalResponseData(responseData);
							}
						});
				}
				this->mLocalMessageQueue.pop();
			}
		}
	}

	bool ServiceManager::PushRequestMessage(SharedPacket messageData)
	{
		const std::string & service = messageData->service();
		ServiceBase * localService = this->GetService(service);
		if (localService == nullptr)
		{
			return false;
		}
		const std::string & method = messageData->method();
		if (!localService->HasMethod(method))
		{
			return false;
		}
		this->mLocalMessageQueue.push(messageData);
		return true;
	}

	bool ServiceManager::PushRequestMessage(const std::string & address, SharedPacket messageData)
	{
		const std::string & service = messageData->service();
		ServiceBase * localService = this->GetService(service);
		if (localService == nullptr)
		{
			return false;
		}
		const std::string & method = messageData->method();
		if (!localService->HasMethod(method))
		{
			return false;
		}
		this->mRemoteMessageQueue.AddItem(make_shared<NetMessageBuffer>(address, messageData));
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
