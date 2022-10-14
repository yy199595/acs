//
// Created by zmhy0073 on 2022/10/12.
//

#ifndef APP_CLUSTERCOMPONENT_H
#define APP_CLUSTERCOMPONENT_H
#include"Component/Component.h"

namespace Cluster
{
    class Server
    {
    public:
        Server(const std::string & name);
    public:
        bool LoadService(const rapidjson::Value & value);
        bool LoadLocation(const rapidjson::Value & value);
    public:
        bool IsAutoAllot() const { return this->mIsAuthAllot; }
        size_t GetServices(std::vector<std::string> & services, bool start = false) const;
    private:
        std::string mName;
        bool mIsAuthAllot;
        std::vector<std::string> mLocations;
        std::unordered_map<std::string, bool> mServices;
    };
}

namespace Sentry
{
    class ClusterComponent : public Component, public IStart
    {
    public:
        ClusterComponent() = default;
        ~ClusterComponent() = default;

    public:
        Cluster::Server * GetServer(const std::string & name);
    private:
        bool Awake() final;
        bool Start() final;
        bool CreateServices(const std::vector<std::string> & services);
    private:
        std::unordered_map<std::string, std::unique_ptr<Cluster::Server>> mServers;
    };
}


#endif //APP_CLUSTERCOMPONENT_H
