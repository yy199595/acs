#pragma once

#include"Component/ServiceBase/ServiceComponentBase.h"
using namespace com;
namespace GameKeeper
{
    class RpcComponent;

    class TaskComponent;

    class ServiceComponent;

    class RedisService : public ServiceComponentBase, public IStart
    {
    public:
		RedisService() = default;

        ~RedisService() override = default;

    public:
        bool RemoveNode(const std::string & address);
        std::vector<std::string> QueryService(const std::string & name);
    public:
        bool Awake() final;

        void OnStart() final;

        bool LateAwake() final;

        int GetPriority() final { return 1000; }


    private:
        int mAreaId;
		std::string mNodeName;
        class RedisComponent * mRedisComponent;
    };
}