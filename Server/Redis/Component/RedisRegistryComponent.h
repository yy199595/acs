#pragma once
#include"XCode/XCode.h"
#include"Time/TimeHelper.h"
#include"Component/RedisChannelComponent.h"
using namespace com;

namespace Sentry
{
    class ServiceNode
    {
    public:
        ServiceNode(const std::string & host)
        {
            this->mHost = host;
            this->UpdateTime();
        }

    public:
        bool RemoveService(const std::string & service);
        long long GetLastTime() const { return this->mLastTime; }
        const std::string & GetHost() const { return this->mHost; }
        void UpdateTime() { this->mLastTime = Helper::Time::GetNowSecTime(); }
        const std::set<std::string> & GetServices() const { return this->mServices; }
        void AddService(const std::string & service) { this->mServices.insert(service); }
    private:
        std::string mHost;
        long long mLastTime;
        std::set<std::string> mServices;
    };
}

namespace Sentry
{

	class TaskComponent;
	class InnerNetMessageComponent;
    class RedisRegistryComponent : public RedisChannelComponent,
            public IComplete, public IServiceChange, public ISecondUpdate
	{
	 public:
		RedisRegistryComponent() = default;
		~RedisRegistryComponent() override = default;
	 public:
		bool LateAwake() final;
		void OnDestory() final;
		void OnComplete() final;
        void OnSecondUpdate(const int tick) final;
	 private:
		void OnAddService(const std::string & name) final;
		void OnDelService(const std::string & name) final;
    private:
        void QueryAllNodes();
        bool Ping(const Json::Reader & json);
        bool AddNode(const Json::Reader & json);
        bool AddService(const Json::Reader & json);
        bool RemoveService(const Json::Reader & json);
        bool OnRegisterEvent(NetEventRegistry &eventRegister) final;
	 private:
		int mAreaId;
		std::string mRpcAddress;
		std::vector<std::string> mServices;
        class TaskComponent * mTaskComponent;
        class RedisDataComponent * mRedisComponent;
        std::unordered_map<std::string, ServiceNode *> mNodes;
	};
}