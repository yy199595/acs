#pragma once

#include"Service/SubService.h"
using namespace com;
namespace Sentry
{
    class RpcComponent;

    class TaskComponent;

    class ServiceProxyComponent;

    class LocalService : public SubService, public IStart
    {
    public:
		LocalService() = default;
        ~LocalService() override = default;

    public:
        bool Awake() final;

        bool LateAwake() final;

        void OnStart() final;

    public:
        void RemoveByAddress(const std::string & address);

    private:
        void Add(const RapidJsonReader & jsonReader);
        void Push(const RapidJsonReader & jsonReader);
        void Remove(const RapidJsonReader & jsonReader);
        bool GetServiceInfo(RapidJsonWriter & jsonWriter);
    private:
        int mAreaId;
		std::string mNodeName;
        class RedisComponent * mRedisComponent;
        class HttpClientComponent * mHttpComponent;
        class ServiceProxyComponent * mServiceComponent;
        std::unordered_map<std::string, std::list<std::string>> mAddressMap;
    };
}