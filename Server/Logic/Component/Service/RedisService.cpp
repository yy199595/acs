#include "RedisService.h"
#include "Core/App.h"
#include "Service/ServiceEntity.h"
#include "Component/Scene/ServiceComponent.h"
#include"Network/Listener/NetworkListener.h"
#include"Network/Listener/TcpServerComponent.h"
#include"Scene/OperatorComponent.h"
#include"Component/Scene/RpcConfigComponent.h"
#include"Util/JsonHelper.h"
#include"RedisComponent.h"
namespace GameKeeper
{
    bool RedisService::Awake()
    {
        LOG_CHECK_RET_FALSE(App::Get().GetConfig().GetValue("area_id", this->mAreaId));
        LOG_CHECK_RET_FALSE(App::Get().GetConfig().GetValue("node_name", this->mNodeName));
        return true;
    }

    bool RedisService::LateAwake()
    {
        LOG_CHECK_RET_FALSE(this->mRedisComponent = this->GetComponent<RedisComponent>());
        return true;
    }

	void RedisService::OnStart()
    {
        std::vector<const NetworkListener *> listeners;
        auto tcpServerComponent = this->GetComponent<TcpServerComponent>();

        const NetworkListener *rpcListener = tcpServerComponent->GetListener("rpc");
        if (rpcListener == nullptr)
        {
            return;
        }
        std::vector<Component *> components;
        this->gameObject->GetComponents(components);

        std::vector<std::string> services
        {
            std::to_string(this->mAreaId),
            rpcListener->GetConfig().mAddress,
        };
        for(Component * component : components)
        {
            ServiceComponentBase *serviceComponent = dynamic_cast<ServiceComponentBase *>(component);
            if (serviceComponent == nullptr)
            {
                continue;
            }
            services.emplace_back(serviceComponent->GetServiceName());
        }
        auto response = this->mRedisComponent->Call("Service", "Add", services);
        if(response != nullptr && response->HasError())
        {
            LOG_WARN("register all service to redis Successful");
        }
    }

    bool RedisService::AddService(const std::vector<std::string> &services)
    {

    }

    bool RedisService::RemoveNode(const std::string &address)
    {
        auto response = this->mRedisComponent->Call("Service", "Remove", this->mAreaId, address);
        if(response->HasError())
        {
            return false;
        }
        LOG_WARN("remove [", address,"] count = ", response->GetNumber());
        return true;
    }

    std::vector<std::string> RedisService::QueryService(const std::string &name)
    {
        auto response = this->mRedisComponent->Call("Service", "Get", this->mAreaId, name);
        if(response->HasError())
        {
            std::vector<std::string> nodeList;
            for(int index = 0; index < response->GetArraySize(); index++)
            {
                nodeList.emplace_back(std::move(response->GetValue(index)));
            }
            return nodeList;
        }
        return std::vector<std::string>();
    }
}// namespace GameKeeper