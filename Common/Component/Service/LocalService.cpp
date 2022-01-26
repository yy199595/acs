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
        LOG_CHECK_RET_FALSE(App::Get().GetConfig().GetValue("area_id", this->mAreaId));
        LOG_CHECK_RET_FALSE(App::Get().GetConfig().GetValue("node_name", this->mNodeName));
        return true;
    }

    bool LocalService::LateAwake()
    {
        LOG_CHECK_RET_FALSE(this->mRedisComponent = this->GetComponent<RedisComponent>());
        return true;
    }

    void LocalService::Add(const RapidJsonReader &jsonReader)
    {

    }

    void LocalService::Push(const RapidJsonReader &jsonReader)
    {
        string ip;
        int port = 0;
        std::vector<std::string> services;
        LOG_CHECK_RET(jsonReader.TryGetValue("rpc", "ip", ip));
        LOG_CHECK_RET(jsonReader.TryGetValue("rpc", "port", port));
        LOG_CHECK_RET(jsonReader.TryGetValue("rpc", "service", services));
        ServiceProxyComponent * proxyComponent = this->GetComponent<ServiceProxyComponent>();
        HttpClientComponent * httpClientComponent = this->GetComponent<HttpClientComponent>();
        for(const std::string & service : services)
        {
            auto serviceProxy = proxyComponent->GetServiceProxy(service);
            serviceProxy->AddAddress(fmt::format("{0}:{1}", ip, port));
        }
        RapidJsonWriter jsonWriter;
        LOG_CHECK_RET(this->GetServiceInfo(jsonWriter));
        if(this->GetServiceInfo(jsonWriter))
        {
            LOG_CHECK_RET(jsonReader.TryGetValue("http", "ip", ip));
            LOG_CHECK_RET(jsonReader.TryGetValue("http", "port", port));
            string url = fmt::format("http://{0}:{1}/logic/service/push", ip, port);
            if (httpClientComponent->Post(url, jsonWriter) != nullptr)
            {
                LOG_INFO("post service to ", ip, ':', port, " successful");
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

    bool LocalService::GetServiceInfo(RapidJsonWriter &jsonWriter)
    {
        auto tcpServerComponent = this->GetComponent<TcpServerComponent>();
        const NetworkListener *rpcListener = tcpServerComponent->GetListener("rpc");
        const NetworkListener *httpListener = tcpServerComponent->GetListener("http");
        LOG_CHECK_RET_FALSE(rpcListener != nullptr && httpListener != nullptr);

        jsonWriter.Add("rpc");
        jsonWriter.StartObject();
        jsonWriter.Add("ip", rpcListener->GetConfig().Ip);
        jsonWriter.Add("port", (int)rpcListener->GetConfig().Port);

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
        jsonWriter.Add("ip", httpListener->GetConfig().Ip);
        jsonWriter.Add("port", httpListener->GetConfig().Port);
        jsonWriter.EndObject();

        jsonWriter.Add("area_id", this->mAreaId);
        jsonWriter.Add("node_name", this->mNodeName);
        return true;
    }
}// namespace Sentry