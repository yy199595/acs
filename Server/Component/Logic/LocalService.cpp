﻿#include"LocalService.h"
#include"App/App.h"
#include"Component/Service/RpcService.h"
#include"Service/ServiceProxy.h"
#include"Component/Scene/ServiceMgrComponent.h"
#include"Network/Listener/NetworkListener.h"
#include"Network/Listener/TcpServerComponent.h"
#include"Component/Redis/RedisComponent.h"
#include"Component/Http/HttpClientComponent.h"
#include"Network/Http/HttpAsyncRequest.h"
namespace Sentry
{
    bool LocalService::Awake()
    {
        BIND_SUB_FUNCTION(LocalService::Add);
        BIND_SUB_FUNCTION(LocalService::Push);
        BIND_SUB_FUNCTION(LocalService::Remove);
        const ServerConfig & config = App::Get()->GetConfig();
        LOG_CHECK_RET_FALSE(config.GetMember("area_id", this->mAreaId));
        LOG_CHECK_RET_FALSE(config.GetMember("node_name", this->mNodeName));
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

    void LocalService::Add(const Json::Reader &jsonReader)
    {
        int areaId = 0;
        std::string address;
        std::string service;
        LOG_CHECK_RET(jsonReader.GetMember("area_id", areaId));
        LOG_CHECK_RET(jsonReader.GetMember("address", address));
        LOG_CHECK_RET(jsonReader.GetMember("service", service));
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
        Json::Writer jsonWriter;
        jsonWriter.AddMember("address", address);
        this->mRedisComponent->Publish("LocalService.Remove", jsonWriter);
    }

    void LocalService::Push(const Json::Reader &jsonReader)
    {
        int areaId = 0;
        LOG_CHECK_RET(jsonReader.GetMember("area_id", areaId));
        if (areaId != 0 && areaId != this->mAreaId) {
            return;
        }
        std::string rpcAddress;
        std::string httpAddress;
        std::vector<std::string> services;
        LOG_CHECK_RET(jsonReader.GetMember("rpc", "service", services));
        LOG_CHECK_RET(jsonReader.GetMember("rpc", "address", rpcAddress));
        LOG_CHECK_RET(jsonReader.GetMember("http", "address", httpAddress));

        for (const std::string &service: services) {
            auto serviceProxy = this->mServiceComponent->GetServiceProxy(service);
            if (serviceProxy != nullptr) {
                serviceProxy->AddAddress(rpcAddress);
            }
        }
        this->mAddressMap.emplace(rpcAddress, services);

        ElapsedTimer elapsedTimer;
        Json::Writer jsonWriter;
        this->GetServiceInfo(jsonWriter);
        string url = fmt::format("http://{0}/logic/service/push", httpAddress);
        std::shared_ptr<HttpAsyncResponse> httpResponse = this->mHttpComponent->Post(url, jsonWriter);
        if (httpResponse != nullptr) {
            LOG_INFO("post service to {0} successful {1}ms", httpAddress, elapsedTimer.GetMs());
        }
    }

    void LocalService::OnStart() //通知其他服务器 我加入了
    {
        Json::Writer jsonWriter;
        this->GetServiceInfo(jsonWriter);
        long long number = this->mRedisComponent->Publish("LocalService.Push", jsonWriter);
        LOG_DEBUG("publish successful count = {0}", number);
    }

    void LocalService::OnDestory()
    {
        std::string jsonContent;
        Json::Writer jsonWriter;
        auto tcpServerComponent = this->GetComponent<TcpServerComponent>();
        const NetworkListener *rpcListener = tcpServerComponent->GetListener("rpc");
        LOG_CHECK_RET(rpcListener);
        jsonWriter.AddMember("area_id", this->mAreaId);
        const ListenConfig &rpcConfig = rpcListener->GetConfig();
        jsonWriter.AddMember("address", fmt::format("{0}:{1}", rpcConfig.Ip, rpcConfig.Port));
        long long number = this->mRedisComponent->Publish("LocalService.Remove", jsonWriter);
        LOG_DEBUG("remove this form count = ", number);

    }

    bool LocalService::AddNewService(const std::string &name)
    {
        Component *component = this->GetComponent<Component>(name);
        if(component != nullptr)
        {
            return false;
        }
        component = ComponentFactory::CreateComponent(name);
        if(component == nullptr || dynamic_cast<RpcService*>(component) == nullptr)
        {
            delete component;
            return false;
        }
        if(!this->mEntity->AddComponent(name, component) || component->LateAwake())
        {
            this->mEntity->RemoveComponent(name);
            return false;
        }

        if (this->GetComponent<RpcService>(name) != nullptr)
        {
            Json::Writer jsonWriter;
            jsonWriter.AddMember("service", name);
            jsonWriter.AddMember("area_id", this->mAreaId);
            jsonWriter.AddMember("address", this->mRpcAddress);
            this->mRedisComponent->Publish("LocalService.Save", jsonWriter);
        }
        LOG_INFO("start new component [", name, "] successful");
        return true;
    }

    void LocalService::Remove(const Json::Reader &jsonReader)
    {
        std::string address;
        LOG_CHECK_RET(jsonReader.GetMember("address", address));

        auto iter = this->mAddressMap.find(address);
        if(iter == this->mAddressMap.end())
        {
            LOG_WARN("not find address ", address);
            return;
        }
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

    void LocalService::GetServiceInfo(Json::Writer &jsonWriter)
    {
        jsonWriter.StartObject("rpc");
        jsonWriter.AddMember("address", this->mRpcAddress);

        std::vector<std::string> tempArray;
        std::list<RpcService *> rpcServices;
        App::Get()->GetTypeComponents(rpcServices);
        for(RpcService * rpcService : rpcServices)
        {
            tempArray.emplace_back(rpcService->GetName());
        }
        jsonWriter.AddMember("service", tempArray);
        jsonWriter.EndObject();

        jsonWriter.StartObject("http");
        jsonWriter.AddMember("address", this->mHttpAddress);
        jsonWriter.EndObject();

        jsonWriter.AddMember("area_id", this->mAreaId);
        jsonWriter.AddMember("node_name", this->mNodeName);
    }
}// namespace Sentry