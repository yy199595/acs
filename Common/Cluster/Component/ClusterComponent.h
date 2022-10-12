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
        const std::vector<std::string> & GetServices() const { return this->mServices; }
        const std::vector<std::string> & GetLocations() const { return this->mLocations; }
    private:
        std::string mName;
        bool mIsAuthAllot;
        std::vector<std::string> mServices;
        std::vector<std::string> mLocations;
    };
}

namespace Sentry
{
    class ClusterComponent : public Component
    {
    public:
        ClusterComponent() = default;
        ~ClusterComponent() = default;
    private:
        void Awake() final;

    private:
        std::unordered_map<std::string, Cluster::Server *> mServers;
    };
}


#endif //APP_CLUSTERCOMPONENT_H
