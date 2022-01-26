#include"NodeService.h"
#include"Object/App.h"
#include"Service/RcpService.h"
#include"Service/ServiceProxy.h"
#include"Scene/ServiceProxyComponent.h"
#include"Network/Listener/NetworkListener.h"
#include"Network/Listener/TcpServerComponent.h"
#include"Util/JsonHelper.h"
#include"RedisComponent.h"
#include"Http/Component/HttpClientComponent.h"
namespace Sentry
{
    struct NetAddressInfo
    {
        string Ip;
        int Port;
    };

    bool NodeService::Awake()
    {
        BIND_SUB_FUNCTION(NodeService::Add);
        BIND_SUB_FUNCTION(NodeService::Register);
        LOG_CHECK_RET_FALSE(App::Get().GetConfig().GetValue("area_id", this->mAreaId));
        LOG_CHECK_RET_FALSE(App::Get().GetConfig().GetValue("node_name", this->mNodeName));
        return true;
    }

    bool NodeService::LateAwake()
    {
        LOG_CHECK_RET_FALSE(this->mRedisComponent = this->GetComponent<RedisComponent>());
        return true;
    }

    void NodeService::Add(const RapidJsonReader &jsonReader)
    {

    }

    void NodeService::Register(const RapidJsonReader &jsonReader)
    {
        NetAddressInfo rpcAddressInfo;
        NetAddressInfo httpAddressInfo;
        std::vector<std::string> services;
        LOG_CHECK_RET(jsonReader.TryGetValue("rpc", "service", services));
        LOG_CHECK_RET(jsonReader.TryGetValue("rpc", "ip", rpcAddressInfo.Ip));
        LOG_CHECK_RET(jsonReader.TryGetValue("http", "ip", rpcAddressInfo.Ip));
        LOG_CHECK_RET(jsonReader.TryGetValue("rpc", "port", rpcAddressInfo.Port));
        LOG_CHECK_RET(jsonReader.TryGetValue("http", "port", rpcAddressInfo.Ip));
        ServiceProxyComponent * proxyComponent = this->GetComponent<ServiceProxyComponent>();
        HttpClientComponent * httpClientComponent = this->GetComponent<HttpClientComponent>();
        for(const std::string & service : services)
        {
            std::shared_ptr<ServiceProxy> serviceProxy = proxyComponent->GetServiceProxy(service);
            serviceProxy->AddAddress(fmt::format("{0}:{1}", rpcAddressInfo.Ip, rpcAddressInfo.Port));
        }
        RapidJsonWriter jsonWriter;
        if(this->GetServiceInfo(jsonWriter))
        {
            std::string content;
            jsonWriter.WriterToStream(content);
            string url = fmt::format("http://{0}:{1}/logic/service/add");
            httpClientComponent->Post(url, content);
        }
    }

    void NodeService::OnStart() //把本机服务注册到redis
    {
        this->RegisterService();
    }

    bool NodeService::GetServiceInfo(RapidJsonWriter &jsonWriter)
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
        std::list<RcpService *> rpcServices;
        App::Get().GetTypeComponents(rpcServices);
        for(RcpService * rpcService : rpcServices)
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

    void NodeService::RegisterService()
    {
        RapidJsonWriter jsonWriter;
        if(this->GetServiceInfo(jsonWriter))
        {
            std::string jsonContent;
            jsonWriter.WriterToStream(jsonContent);
            long long number = this->mRedisComponent->Publish("NodeService.Add", jsonContent);
            LOG_DEBUG("publish successful count = ", number);
        }
    }

    void NodeService::OnDestory()
    {
        auto tcpServerComponent = this->GetComponent<TcpServerComponent>();
        const NetworkListener *rpcListener = tcpServerComponent->GetListener("rpc");
        if (rpcListener == nullptr)
        {
            return;
        }
        this->RemoveNode(rpcListener->GetConfig().mAddress);
    }

    bool NodeService::RemoveNode(const std::string &address)
    {
        auto response = this->mRedisComponent->Call("Service", "Remove", this->mAreaId, address);
        if(response->HasError())
        {
            return false;
        }
        LOG_WARN("remove [", address,"] count = ", response->GetNumber());
        return true;
    }
}// namespace Sentry