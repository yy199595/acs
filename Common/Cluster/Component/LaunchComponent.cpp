//
// Created by zmhy0073 on 2022/10/12.
//

#include"LaunchComponent.h"
#include"App/App.h"
#include"File/FileHelper.h"
#include"System/System.h"
#include"Config/ClusterConfig.h"
#include"Service/ServiceRpcComponent.h"
#include"Service/LuaRpcService.h"
#include"Service/LocalRpcService.h"
#include"Service/LocalHttpService.h"
#include"Service/LuaHttpService.h"
#include"Component/LocationComponent.h"
namespace Sentry
{
    bool LaunchComponent::Awake()
    {
        const NodeConfig * nodeConfig = ClusterConfig::Inst()->GetConfig();
		LOG_CHECK_RET_FALSE(nodeConfig);
        std::vector<std::string> components;
        if(nodeConfig->GetComponents(components))
        {
            for(const std::string & name : components)
            {
                if(!this->mApp->AddComponent(name))
                {
                    LOG_ERROR("add " << name << " error");
                    return false;
                }
                //CONSOLE_LOG_INFO(ServerConfig::Inst()->Name() << " add component [" << name << "]");
            }
        }
        components.clear();
        if(nodeConfig->GetServices(components) > 0)
        {
            for(const std::string & name : components)
            {
                const RpcServiceConfig * rpcServiceConfig = RpcConfig::Inst()->GetConfig(name);
                const HttpServiceConfig * httpServiceConfig = HttpConfig::Inst()->GetConfig(name);
                if(!this->mApp->AddComponent(name))
                {
                    if(rpcServiceConfig != nullptr)
                    {
                        if(nodeConfig->IsStart(name)) //使用lua启动
                        {
                            std::unique_ptr<Component> component(new LuaRpcService());
                            if(!this->mApp->AddComponent(name, std::move(component)))
                            {
                                LOG_ERROR("add start rpc service [" << name << "] error");
                                return false;
                            }
                        }
                        else
                        {
                            std::unique_ptr<Component> component(new ServiceRpcComponent());
                            if(!this->mApp->AddComponent(name, std::move(component)))
                            {
                                LOG_ERROR("add not start rpc service [" << name << "] error");
                                return false;
                            }
                        }
                    }
                    else if(httpServiceConfig != nullptr && nodeConfig->IsStart(name))
                    {
                        std::unique_ptr<Component> component(new LuaHttpService());
                        if(!this->mApp->AddComponent(name, std::move(component)))
                        {
                            LOG_ERROR("add http service [" << name << "] error");
                            return false;
                        }
                    }
					else
					{
						return false;
					}
                }
                if (nodeConfig->IsStart(name))
                {                  
                    this->GetComponent<IServiceBase>(name)->Init(); 
                    //CONSOLE_LOG_INFO(ServerConfig::Inst()->Name() << " start service [" << name << "]");
                }              
            }
        }
        return true;
    }

    bool LaunchComponent::Start()
    {
        const ServerConfig * config = ServerConfig::Inst();
        const ClusterConfig * clusterConfig  = ClusterConfig::Inst();
        LocationComponent * locationComponent = this->GetComponent<LocationComponent>();

		std::string location, httpLocation;
		std::vector<std::string> components;
		config->GetLocation("rpc", location);
		config->GetLocation("http", httpLocation);
        if(clusterConfig->GetConfig()->GetServices(components, true) > 0)
        {
            for(const std::string & name : components)
            {
#ifdef __DEBUG__
                long long t1 = Helper::Time::NowMilTime();
#endif
                if(!this->GetComponent<IServiceBase>(name)->Start())
                {
                    LOG_ERROR("start service [" << name << "] faillure");
                    return false;
                }
                if(RpcConfig::Inst()->GetConfig(name) != nullptr)
                {
					LOG_CHECK_RET_FALSE(!location.empty());
#ifdef __DEBUG__
                    long long t2 = Helper::Time::NowMilTime();
                    LOG_INFO("start rpc service [" << name << "] successful use time [" << t2 - t1 << "ms]");
#else
                    LOG_INFO("start rpc service [" << name << "] successful");
#endif
                }
                else if(HttpConfig::Inst()->GetConfig(name) != nullptr)
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
		locationComponent->AddRpcServer(ServerConfig::Inst()->Name(), location);
		locationComponent->AddHttpServer(ServerConfig::Inst()->Name(), httpLocation);
        return true;
    }
}