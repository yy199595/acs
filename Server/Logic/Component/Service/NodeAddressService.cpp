#include"NodeAddressService.h"
#include"Core/App.h"
#include"Service/ServiceEntity.h"
#include"Component/Scene/ServiceComponent.h"
#include"Network/Listener/NetworkListener.h"
#include"Network/Listener/TcpServerComponent.h"
#include"Util/JsonHelper.h"
#include"RedisComponent.h"
namespace GameKeeper
{
    bool NodeAddressService::Awake()
    {
        BIND_SUB_FUNCTION(NodeAddressService::Add);
        LOG_CHECK_RET_FALSE(App::Get().GetConfig().GetValue("area_id", this->mAreaId));
        LOG_CHECK_RET_FALSE(App::Get().GetConfig().GetValue("node_name", this->mNodeName));
        return true;
    }

    bool NodeAddressService::LateAwake()
    {
        LOG_CHECK_RET_FALSE(this->mRedisComponent = this->GetComponent<RedisComponent>());
        return true;
    }

    void NodeAddressService::Add(const RapidJsonReader &jsonReader)
    {
        int areaId = 0;
        if(jsonReader.TryGetValue("area_id", areaId))
        {
            if(areaId != 0 && areaId != this->mAreaId)
            {
                return;
            }
            std::string address;
            std::vector<std::string> services;
            jsonReader.TryGetValue("address", address);
            jsonReader.TryGetValue("services", services);
            ServiceComponent * serviceComponent = this->GetComponent<ServiceComponent>();
            if(serviceComponent != nullptr)
            {
                for(const std::string & name : services)
                {
                   auto serviceEntity = serviceComponent->GetServiceEntity(name);
                   if(serviceEntity != nullptr)
                   {
                       serviceEntity->AddAddress(address);
                   }
                }
            }
        }
    }

    void NodeAddressService::Remove(const RapidJsonReader &jsonReader)
    {

    }

    void NodeAddressService::OnStart() //把本机服务注册到redis
    {
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
        if(response != nullptr && !response->HasError())
        {
            LOG_WARN("register all service to redis Successful");
        }

        auto queryResponse = this->mRedisComponent->Call("Service", "Get", this->mAreaId);
        if(queryResponse != nullptr && !queryResponse->HasError())
        {
            RapidJsonReader jsonReader;
            for(size_t index = 0; index < queryResponse->GetArraySize(); index++)
            {
                const std::string & json = queryResponse->GetValue(index);
                if(jsonReader.TryParse(json))
                {
                    this->Add(jsonReader);
                }
            }
        }
    }

    bool NodeAddressService::RemoveNode(const std::string &address)
    {
        auto response = this->mRedisComponent->Call("Service", "Remove", this->mAreaId, address);
        if(response->HasError())
        {
            return false;
        }
        LOG_WARN("remove [", address,"] count = ", response->GetNumber());
        return true;
    }
}// namespace GameKeeper