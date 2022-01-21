#pragma once

#include"Service/ServiceComponentBase.h"
using namespace com;
namespace Sentry
{
    class RpcComponent;

    class TaskComponent;

    class ServiceProxyComponent;

    class NodeAddressService : public ServiceComponentBase, public IStart
    {
    public:
		NodeAddressService() = default;

        ~NodeAddressService() override = default;

    public:
        bool RemoveNode(const std::string & address);
    public:
        bool Awake() final;

        bool LateAwake() final;

        void OnStart() final;

        void OnDestory() final;
    private:
        void Add(const RapidJsonReader & jsonReader);
        void Remove(const RapidJsonReader & jsonReader);

    private:
        int mAreaId;
		std::string mNodeName;
        class RedisComponent * mRedisComponent;
    };
}