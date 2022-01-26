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
        bool GetServiceInfo(RapidJsonWriter & jsonWriter);
    private:
        void Add(const RapidJsonReader & jsonReader);
        void Push(const RapidJsonReader & jsonReader);

    private:
    private:
        int mAreaId;
		std::string mNodeName;
        class RedisComponent * mRedisComponent;
        class HttpClientComponent * mHttpComponent;
        class ServiceProxyComponent * mServiceComponent;
    };
}