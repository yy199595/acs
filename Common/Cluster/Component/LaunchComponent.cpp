//
// Created by zmhy0073 on 2022/10/12.
//

#include"LaunchComponent.h"
#include"Entity/Unit/App.h"
#include"Util/File/FileHelper.h"
#include"Core/System/System.h"
#include"Http/Component/HttpComponent.h"
#include"Cluster//Config/ClusterConfig.h"
#include"Rpc/Service/LuaPhysicalRpcService.h"
#include"Http//Service/LuaPhysicalHttpService.h"
#include"Rpc/Component/LocationComponent.h"
#include"Lua/Component/LuaScriptComponent.h"
#include"Rpc/Component/InnerRpcComponent.h"
#include"Rpc/Component/InnerNetComponent.h"
#include"Http/Component/HttpWebComponent.h"
#include"Rpc/Component/DispatchComponent.h"
namespace Tendo
{
    bool LaunchComponent::Awake()
    {
        this->mApp->AddComponent<HttpComponent>();
		this->mApp->AddComponent<InnerRpcComponent>();
		if(ServerConfig::Inst()->UseLua())
		{
			this->mApp->AddComponent<LuaScriptComponent>();
		}
		unsigned short port = 0;
		if(ServerConfig::Inst()->GetListen("rpc", port))
		{
			this->mApp->AddComponent<LocationComponent>();
			this->mApp->AddComponent<InnerNetComponent>();
			this->mApp->AddComponent<DispatchComponent>();
		}
		if(ServerConfig::Inst()->GetListen("http", port))
		{
			this->mApp->AddComponent<LocationComponent>();
			this->mApp->AddComponent<HttpWebComponent>();
			this->mApp->AddComponent<DispatchComponent>();
		}

        std::vector<std::string> components;
		const NodeConfig* nodeConfig = ClusterConfig::Inst()->GetConfig();
        if (nodeConfig->GetComponents(components))
		{
			for (const std::string& name : components)
			{
				if(!this->mApp->HasComponent(name))
				{
					if(!this->mApp->AddComponent(name))
					{
						LOG_ERROR("add " << name << " error");
						return false;
					}
				}
				//CONSOLE_LOG_INFO(ServerConfig::Inst()->Name() << " add component [" << name << "]");
			}
		}
        components.clear();
        if (nodeConfig->GetServices(components) > 0)
        {
            for (const std::string& name : components)
            {
                const RpcServiceConfig* rpcServiceConfig = RpcConfig::Inst()->GetConfig(name);
                const HttpServiceConfig* httpServiceConfig = HttpConfig::Inst()->GetConfig(name);
                if (rpcServiceConfig != nullptr)
				{
					//创建实体服务
					if (!this->mApp->AddComponent(name))
					{
						std::unique_ptr<Component> component(new LuaPhysicalRpcService());
						if (!this->mApp->AddComponent(name, std::move(component)))
						{
							LOG_ERROR("add physical service [" << name << "] error");
							return false;
						}
					}
				}
                else if(httpServiceConfig != nullptr)
                {
                    if (!this->mApp->AddComponent(name))
                    {
                        std::unique_ptr<Component> component(new LuaPhysicalHttpService());
                        if (!this->mApp->AddComponent(name, std::move(component)))
                        {
                            LOG_ERROR("add http service [" << name << "] error");
                            return false;
                        }
                    }
                }
                else
                {
                    LOG_ERROR("not find service config [" << name << "]");
                    return false;
                }
            }
        }
        return true;
    }

    void LaunchComponent::Start()
    {
		std::vector<IServiceBase *> components;
		this->mApp->GetComponents<IServiceBase>(components);
		for(IServiceBase * localService : components)
		{
#ifdef __DEBUG__
			long long t1 = Helper::Time::NowMilTime();
#endif
			Component * component = dynamic_cast<Component*>(localService);
			{
				localService->Start();
			}
#ifdef __DEBUG__
			long long t2 = Helper::Time::NowMilTime();
			CONSOLE_LOG_INFO("start" << component->GetName() << " time = [" << (t2 - t1) << "ms]")
#endif
		}
    }

	bool LaunchComponent::LateAwake()
	{
		std::vector<IServiceBase *> allServices;
		this->mApp->GetComponents(allServices);
		for(IServiceBase * service : allServices)
		{
			if(!service->Init())
			{
				return false;
			}
		}
		return true;
	}

	void LaunchComponent::OnDestroy()
	{
		std::vector<IServiceBase *> allServices;
		this->mApp->GetComponents(allServices);
		for(IServiceBase * service : allServices)
		{
			service->Close();
#ifdef __DEBUG__
			Component* component = dynamic_cast<Component*>(service);
			CONSOLE_LOG_INFO(component->GetName() << ".OnClose");
#endif
		}
		this->mApp->GetCoroutine()->Sleep(2000); //等待处理的消息返回
	}
}