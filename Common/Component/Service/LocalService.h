#pragma once

#include"Service/SubService.h"
using namespace com;
namespace Sentry
{
    class RpcComponent;

    class TaskComponent;

    class ServiceMgrComponent;

    class LocalService : public SubService, public IStart
    {
    public:
		LocalService() = default;
        ~LocalService() override = default;

    public:
        bool Awake() final;

        bool LateAwake() final;

        void OnStart() final;

        void OnDestory() final;
    public:
        bool AddNewService(const std::string & service);
        void RemoveByAddress(const std::string & address);
    private:
        void Add(const RapidJsonReader & jsonReader);
        void Push(const RapidJsonReader & jsonReader);
        void Remove(const RapidJsonReader & jsonReader);
        void GetServiceInfo(RapidJsonWriter & jsonWriter);
    private:
        int mAreaId;
		std::string mNodeName;
        std::string mRpcAddress;
        std::string mHttpAddress;
        class RedisComponent * mRedisComponent;
        class HttpClientComponent * mHttpComponent;
        class ServiceMgrComponent * mServiceComponent;
        class TcpServerComponent * mTcpServerComponent;
        std::unordered_map<std::string, std::vector<std::string>> mAddressMap;
    };
}