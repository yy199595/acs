//
// Created by zmhy0073 on 2022/10/12.
//

#include"ClusterComponent.h"
#include"App/App.h"
#include"File/FileHelper.h"
#include"App/System/System.h"
#include"Config/ClusterConfig.h"
#include"Service/ServiceRpcComponent.h"
#include"Service/LuaRpcService.h"
#include"Service/LocalRpcService.h"
#include"Service/LocalHttpService.h"
#include"Service/LocalLuaHttpService.h"
#include"Component/LocationComponent.h"
namespace Sentry
{
    bool ClusterComponent::Awake()
    {
        const NodeConfig * nodeConfig = ClusterConfig::Inst()->GetConfig();

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
                CONSOLE_LOG_INFO(System::GetName() << " add component [" << name << "]");
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
                        std::unique_ptr<Component> component(new LocalLuaHttpService());
                        if(!this->mApp->AddComponent(name, std::move(component)))
                        {
                            LOG_ERROR("add http service [" << name << "] error");
                            return false;
                        }
                    }
                }
                CONSOLE_LOG_INFO(System::GetName() << " add service [" << name << "]");
            }
        }
        return true;
    }

    bool ClusterComponent::Start()
    {
        const ServerConfig * config = ServerConfig::Inst();
        const ClusterConfig * clusterConfig  = ClusterConfig::Inst();
        LocationComponent * locationComponent = this->GetComponent<LocationComponent>();

        std::vector<std::string> components;
        if(clusterConfig->GetConfig()->GetServices(components, true) > 0)
        {
            std::string location, httpLocation;
            LOG_CHECK_RET_FALSE(config->GetLocation("rpc", location));
            for(const std::string & name : components)
            {
                IServiceBase * component = this->GetComponent<IServiceBase>(name);
                LocalRpcService * localService = dynamic_cast<LocalRpcService*>(component);
                LocalHttpService * localHttpService = dynamic_cast<LocalHttpService*>(component);
                if(component != nullptr && !component->Start())
                {
                    LOG_ERROR("start service [" << name << "] faillure");
                    return false;
                }
                if(localService != nullptr)
                {
                    locationComponent->AddLocation(name, location);
                    CONSOLE_LOG_ERROR("start rpc service [" << name << "] successful");
                }
                else if(localHttpService != nullptr)
                {
                    CONSOLE_LOG_ERROR("start http service [" << name << "] successful");
                }
            }
        }
        return true;
    }
}