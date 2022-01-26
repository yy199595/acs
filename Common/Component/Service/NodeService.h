#pragma once

#include"Service/SubService.h"
using namespace com;
namespace Sentry
{
    class RpcComponent;

    class TaskComponent;

    class ServiceProxyComponent;

    class NodeService : public SubService, public IStart
    {
    public:
		NodeService() = default;
        ~NodeService() override = default;

    public:
        bool RemoveNode(const std::string & address);
    public:
        bool Awake() final;

        bool LateAwake() final;

        void OnStart() final;

        void OnDestory() final;
    private:
        void Add(const RapidJsonReader & jsonReader);
        void Register(const RapidJsonReader & jsonReader);

    private:
        void RegisterService();
        bool GetServiceInfo(RapidJsonWriter & jsonWriter);
    private:
        int mAreaId;
		std::string mNodeName;
        class RedisComponent * mRedisComponent;
    };
}