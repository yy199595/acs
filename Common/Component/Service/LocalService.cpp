#include"LocalService.h"
#include"Object/App.h"
#include"Service/RpcService.h"
#include"Service/ServiceProxy.h"
#include"Scene/ServiceProxyComponent.h"
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
        LOG_CHECK_RET_FALSE(this->mServiceComponent = this->GetComponent<ServiceProxyComponent>());
        return true;
    }

    void LocalService::Add(const RapidJsonReader &jsonReader)
    {

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
        if(areaId != 0 && areaId != this->mAreaId)
        {
            return;
        }
        std::string rpcAddress;
        std::string httpAddress;
        std::vector<std::string> services;
        LOG_CHECK_RET(jsonReader.TryGetValue("rpc", "service", services));
        LOG_CHECK_RET(jsonReader.TryGetValue("rpc", "address", rpcAddress));
        LOG_CHECK_RET(jsonReader.TryGetValue("http", "address", httpAddress));

        for(const std::string & service : services)
        {
            auto serviceProxy = this->mServiceComponent->GetServiceProxy(service);
            if(serviceProxy != nullptr)
            {
                serviceProxy->AddAddress(rpcAddress);
            }
        }
        RapidJsonWriter jsonWriter;
        if(this->GetServiceInfo(jsonWriter))
        {
            ElapsedTimer elapsedTimer;
            string url = fmt::format("http://{0}/logic/service/push", httpAddress);
            std::shared_ptr<HttpAsyncResponse> httpResponse = this->mHttpComponent->Post(url, jsonWriter);
            if(httpResponse!= nullptr)
            {
                LOG_INFO("post service to ", httpAddress, " successful [", elapsedTimer.GetMs(), "ms]");
            }
        }
    }

    void LocalService::OnStart() //通知其他服务器 我加入了
    {
        RapidJsonWriter jsonWriter;
        if(this->GetServiceInfo(jsonWriter))
        {
            std::string jsonContent;
            jsonWriter.WriterToStream(jsonContent);
            long long number = this->mRedisComponent->Publish("LocalService.Push", jsonContent);
            LOG_DEBUG("publish successful count = ", number);
        }
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

    bool LocalService::GetServiceInfo(RapidJsonWriter &jsonWriter)
    {
        auto tcpServerComponent = this->GetComponent<TcpServerComponent>();
        const NetworkListener *rpcListener = tcpServerComponent->GetListener("rpc");
        const NetworkListener *httpListener = tcpServerComponent->GetListener("http");
        LOG_CHECK_RET_FALSE(rpcListener != nullptr && httpListener != nullptr);

        const ListenConfig & rpcConfig = rpcListener->GetConfig();
        const ListenConfig & httpConfig = httpListener->GetConfig();
        const std::string rpcAddress = fmt::format("{0}:{1}", rpcConfig.Ip, rpcConfig.Port);
        const std::string httpAddress = fmt::format("{0}:{1}",httpConfig.Ip, httpConfig.Port);

        jsonWriter.Add("rpc");
        jsonWriter.StartObject();
        jsonWriter.Add("address", rpcAddress);

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
        jsonWriter.Add("address", httpAddress);
        jsonWriter.EndObject();

        jsonWriter.Add("area_id", this->mAreaId);
        jsonWriter.Add("node_name", this->mNodeName);
        return true;
    }
}// namespace Sentry