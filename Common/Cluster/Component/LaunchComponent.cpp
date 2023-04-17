//
// Created by zmhy0073 on 2022/10/12.
//

#include"LaunchComponent.h"
#include"Entity/Unit/App.h"
#include"Util/File/FileHelper.h"
#include"Core/System/System.h"
#include"Http/Component/HttpComponent.h"
#include"Cluster//Config/ClusterConfig.h"
#include"Rpc/Service/VirtualRpcService.h"
#include"Rpc/Service/LuaPhysicalRpcService.h"
#include"Http//Service/LuaPhysicalHttpService.h"
#include"Rpc/Component/NodeMgrComponent.h"
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
			this->mApp->AddComponent<NodeMgrComponent>();
			this->mApp->AddComponent<InnerNetComponent>();
			this->mApp->AddComponent<DispatchComponent>();
		}
		if(ServerConfig::Inst()->GetListen("http", port))
		{
			this->mApp->AddComponent<NodeMgrComponent>();
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
                    if (nodeConfig->IsStart(name))
                    {
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
                    else //创建虚拟服务
                    {
                        std::unique_ptr<Component> component(new VirtualRpcService());
                        if (!this->mApp->AddComponent(name, std::move(component)))
                        {
                            LOG_ERROR("add virtual service [" << name << "] error");
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

    bool LaunchComponent::Start()
    {
        const ServerConfig* config = ServerConfig::Inst();
        const ClusterConfig* clusterConfig = ClusterConfig::Inst();
        TimerComponent* timerComponent = this->GetComponent<TimerComponent>();
        NodeMgrComponent* locationComponent = this->GetComponent<NodeMgrComponent>();

        std::string location, httpLocation;
        std::vector<std::string> components;
        config->GetLocation("rpc", location);
        config->GetLocation("http", httpLocation);
        if (clusterConfig->GetConfig()->GetServices(components, true) > 0)
        {
            for (const std::string& name : components)
            {
#ifdef __DEBUG__
                long long t1 = Helper::Time::NowMilTime();
#endif
                long long id = timerComponent->DelayCall(10 * 1000, [name]()
                {
                    CONSOLE_LOG_ERROR("start service [" << name << "] timeout");
                });
                if (!this->GetComponent<IServiceBase>(name)->Start())
                {
                    LOG_ERROR("start service [" << name << "] failure");
                    return false;
                }
                timerComponent->CancelTimer(id);
                if (RpcConfig::Inst()->GetConfig(name) != nullptr)
                {
                    LOG_CHECK_RET_FALSE(!location.empty());
#ifdef __DEBUG__
                    long long t2 = Helper::Time::NowMilTime();
                    LOG_INFO("start rpc service [" << name << "] successful use time [" << t2 - t1 << "ms]");
#else
                    LOG_INFO("start rpc service [" << name << "] successful");
#endif
                }
                else if (HttpConfig::Inst()->GetConfig(name) != nullptr)
                {
                    long long t2 = Helper::Time::NowMilTime();
                    LOG_CHECK_RET_FALSE(!httpLocation.empty());
#ifdef __DEBUG__
                    LOG_INFO("start http service [" << name << "] successful use time [" << t2 - t1 << "ms]");
#else
                    LOG_INFO("start http service [" << name << "] successful");
#endif
                }
            }
        }
        return true;
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
	}
}