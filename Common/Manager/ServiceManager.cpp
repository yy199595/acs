#include "ServiceManager.h"
#include <Service/LocalLuaService.h>
#include <Service/LocalService.h>

#include <Object/ReflectHelper.h>
#include <Coroutine/CoroutineManager.h>
#include <Manager/ActionManager.h>
#include <Util/StringHelper.h>

namespace Sentry
{
    bool ServiceManager::OnInit()
    {
        SayNoAssertRetFalse_F(this->GetConfig().GetValue("NodeId", this->mNodeId));
        SayNoAssertRetFalse_F(this->GetConfig().GetValue("Service", this->mServiceList));


        SayNoAssertRetFalse_F(this->mActionManager = this->GetManager<ActionManager>());
        SayNoAssertRetFalse_F(this->mCorManager = this->GetManager<CoroutineManager>());
        SayNoAssertRetFalse_F(this->mNetProxyManager = this->GetManager<NetProxyManager>());

        SayNoAssertRetFalse_F(this->CreateLocalService());
        return true;
    }

    void ServiceManager::OnInitComplete()
    {
        std::vector<shared_ptr<LocalActionProxy>> methodVec;
        auto iter = this->mLocalServiceMap.begin();
        for (; iter != this->mLocalServiceMap.end(); iter++)
        {
            LocalService *localService = iter->second;
            localService->GetServiceList(methodVec);
			this->mCorManager->Start(&LocalService::OnInitComplete, localService);
        }
    }

    bool ServiceManager::HandlerMessage(NetMessageProxy *messageData)
    {
        ServiceBase *localService = this->GetService(messageData->GetService());
        if (localService == nullptr || localService->HasMethod(messageData->GetMethd()))
        {
            return false;
        }
		this->mCorManager->Start(&ServiceManager::Invoke1, this, messageData);
        return true;
    }

    bool ServiceManager::HandlerMessage(const std::string &address, NetMessageProxy *messageData)
    {
        const std::string &method = messageData->GetMethd();
        const std::string &service = messageData->GetService();
        ServiceBase *localService = this->GetService(service);
        if (localService == nullptr || !localService->HasMethod(method))
        {
            SayNoDebugError("call function not find [" << service << "." << method << "]");
            return false;
        }
		this->mCorManager->Start(&ServiceManager::Invoke2, this, address, messageData);
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

	void ServiceManager::Invoke1(NetMessageProxy *messageData)
	{

	}

	void ServiceManager::Invoke2(const std::string & address, NetMessageProxy * messageData)
	{
		ServiceBase * localService = this->GetService(messageData->GetService());
		if (localService->InvokeMethod(messageData))
		{
			SayNoDebugError("Invoke " << messageData->GetMethd() << " fail");
			return;
		}
		this->mNetProxyManager->SendMsgByAddress(address, messageData);
	}

	ServiceBase *ServiceManager::GetService(const std::string &name)
    {
        LocalLuaService *luaService = this->GetLuaService(name);
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
        for (size_t index = 0; index < this->mServiceList.size(); index++)
        {
            const std::string &name = this->mServiceList[index];
            LocalService *localService = ReflectHelper<LocalService>::Create(name);
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
            this->mLocalServiceMap.emplace(name, localService);
        }
        return true;
    }
}// namespace Sentry
