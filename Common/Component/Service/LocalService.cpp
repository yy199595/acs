#include"LocalService.h"
#include"Object/App.h"
#include"Service/RpcService.h"
#include"Service/ServiceProxy.h"
#include"Scene/ServiceMgrComponent.h"
#include"Network/Listener/NetworkListener.h"
#include"Network/Listener/TcpServerComponent.h"
#include"Util/JsonHelper.h"
#include"RedisComponent.h"
#include"Http/Component/HttpClientComponent.h"
#include"Http/HttpAsyncRequest.h"
namespace Sentry
{
    struct NetAddressInfo
    {
        string Ip;
        int Port;
    };

    bool LocalService::Awake()
    {
        BIND_SUB_FUNCTION(LocalService::Add);
        BIND_SUB_FUNCTION(LocalService::Push);
        BIND_SUB_FUNCTION(LocalService::Remove);
        LOG_CHECK_RET_FALSE(App::Get().GetConfig().GetValue("area_id", this->mAreaId));
        LOG_CHECK_RET_FALSE(App::Get().GetConfig().GetValue("node_name", this->mNodeName));
        return true;
    }

    bool LocalService::LateAwake()
    {
        LOG_CHECK_RET_FALSE(this->mRedisComponent = this->GetComponent<RedisComponent>());
        LOG_CHECK_RET_FALSE(this->mHttpComponent = this->GetComponent<HttpClientComponent>());
        LOG_CHECK_RET_FALSE(this->mServiceComponent = this->GetComponent<ServiceMgrComponent>());
        LOG_CHECK_RET_FALSE(this->mTcpServerComponent = this->GetComponent<TcpServerComponent>());
        const NetworkListener *rpcListener = this->mTcpServerComponent->GetListener("rpc");
        const NetworkListener *httpListener = this->mTcpServerComponent->GetListener("http");
        LOG_CHECK_RET_FALSE(rpcListener && httpListener);
        const ListenConfig & rpcListenerConfig = rpcListener->GetConfig();
        const ListenConfig & httpListenerConfig = httpListener->GetConfig();
        this->mRpcAddress = fmt::format("{0}:{1}", rpcListenerConfig.Ip, rpcListenerConfig.Port);
        this->mHttpAddress = fmt::format("{0}:{1}", httpListenerConfig.Ip, httpListenerConfig.Port);
        return true;
    }

    void LocalService::Add(const RapidJsonReader &jsonReader)
    {
        int areaId = 0;
        std::string address;
        std::string service;
        LOG_CHECK_RET(jsonReader.TryGetValue("area_id", areaId));
        LOG_CHECK_RET(jsonReader.TryGetValue("address", address));
        LOG_CHECK_RET(jsonReader.TryGetValue("service", service));
        if(areaId != 0 && areaId != this->mAreaId)
        {
            return;
        }
        auto serviceProxy = this->mServiceComponent->GetServiceProxy(service);
        if (serviceProxy != nullptr) {
            serviceProxy->AddAddress(address);
        }
    }

    void LocalService::RemoveByAddress(const std::string &address)
    {
        RapidJsonWriter jsonWriter;
        jsonWriter.Add("address", address);
        this->mRedisComponent->Publish("LocalService.Remove", jsonWriter);
    }

    void LocalService::Push(const RapidJsonReader &jsonReader)
    {
        int areaId = 0;
        LOG_CHECK_RET(jsonReader.TryGetValue("area_id", areaId));
        if (areaId != 0 && areaId != this->mAreaId) {
            return;
        }
        std::string rpcAddress;
        std::string httpAddress;
        std::vector<std::string> services;
        LOG_CHECK_RET(jsonReader.TryGetValue("rpc", "service", services));
        LOG_CHECK_RET(jsonReader.TryGetValue("rpc", "address", rpcAddress));
        LOG_CHECK_RET(jsonReader.TryGetValue("http", "address", httpAddress));

        for (const std::string &service: services) {
            auto serviceProxy = this->mServiceComponent->GetServiceProxy(service);
            if (serviceProxy != nullptr) {
                serviceProxy->AddAddress(rpcAddress);
            }
        }
        this->mAddressMap.emplace(rpcAddress, services);

        ElapsedTimer elapsedTimer;
        RapidJsonWriter jsonWriter;
        this->GetServiceInfo(jsonWriter);
        string url = fmt::format("http://{0}/logic/service/push", httpAddress);
        std::shared_ptr<HttpAsyncResponse> httpResponse = this->mHttpComponent->Post(url, jsonWriter);
        if (httpResponse != nullptr) {
            LOG_INFO("post service to ", httpAddress, " successful [", elapsedTimer.GetMs(), "ms]");
        }

    }

    void LocalService::OnStart() //通知其他服务器 我加入了
    {
        RapidJsonWriter jsonWriter;
        this->GetServiceInfo(jsonWriter);
        long long number = this->mRedisComponent->Publish("LocalService.Push", jsonWriter);
        LOG_DEBUG("publish successful count = ", number);
    }

    void LocalService::OnDestory()
    {
        std::string jsonContent;
        RapidJsonWriter jsonWriter;
        auto tcpServerComponent = this->GetComponent<TcpServerComponent>();
        const NetworkListener *rpcListener = tcpServerComponent->GetListener("rpc");
        LOG_CHECK_RET(rpcListener);
        jsonWriter.Add("area_id", this->mAreaId);
        const ListenConfig &rpcConfig = rpcListener->GetConfig();
        jsonWriter.Add("address", fmt::format("{0}:{1}", rpcConfig.Ip, rpcConfig.Port));
        long long number = this->mRedisComponent->Publish("LocalService.Remove", jsonWriter);
        LOG_DEBUG("remove this form count = ", number);

    }

    bool LocalService::AddNewService(const std::string &service)
    {
        TaskComponent *taskComponent = this->GetComponent<TaskComponent>();
        Component *component = this->GetComponent<Component>(service);
        if (component != nullptr) {
            return false;
        }
        component = ComponentFactory::CreateComponent(service);
        if (component == nullptr) {
            return false;
        }
        if (!this->mEntity->AddComponent(service, component)|| !component->LateAwake()) {
            return false;
        }
        taskComponent->Start([component, this]()
        {
            IStart *startComponent = dynamic_cast<IStart *>(component);
            ILoadData *loadDataComponent = dynamic_cast<ILoadData *>(component);
            if (startComponent != nullptr) {
                startComponent->OnStart();
            }
            if (loadDataComponent != nullptr) {
                loadDataComponent->OnLoadData();
            }
            RapidJsonWriter jsonWriter;
            jsonWriter.Add("area_id", this->mAreaId);
            jsonWriter.Add("address", this->mRpcAddress);
            jsonWriter.Add("service", component->GetTypeName());
            this->mRedisComponent->Publish("LocalService.Add", jsonWriter);
            LOG_INFO("start new service [", component->GetTypeName(), "] successful");
        });
        return true;
    }

    void LocalService::Remove(const RapidJsonReader &jsonReader)
    {
        std::string address;
        LOG_CHECK_RET(jsonReader.TryGetValue("address", address));

        auto iter = this->mAddressMap.find(address);
        LOG_CHECK_RET(iter != this->mAddressMap.end());
        for (const std::string &service: iter->second)
        {
            auto serviceProxy = this->mServiceComponent->GetServiceProxy(service);
            if (serviceProxy != nullptr)
            {
                serviceProxy->RemoveAddress(address);
            }
        }
        this->mAddressMap.erase(iter);
    }

    void LocalService::GetServiceInfo(RapidJsonWriter &jsonWriter)
    {
        jsonWriter.Add("rpc");
        jsonWriter.StartObject();
        jsonWriter.Add("address", this->mRpcAddress);

        std::vector<std::string> tempArray;
        std::list<RpcService *> rpcServices;
        App::Get().GetTypeComponents(rpcServices);
        for(RpcService * rpcService : rpcServices)
        {
            tempArray.emplace_back(rpcService->GetName());
        }
        jsonWriter.Add("service", tempArray);
        jsonWriter.EndObject();

        jsonWriter.Add("http");
        jsonWriter.StartObject();
        jsonWriter.Add("address", this->mHttpAddress);
        jsonWriter.EndObject();

        jsonWriter.Add("area_id", this->mAreaId);
        jsonWriter.Add("node_name", this->mNodeName);
    }
}// namespace Sentry