//
// Created by zmhy0073 on 2022/10/17.
//

#ifndef APP_CLUSTERCONFIG_H
#define APP_CLUSTERCONFIG_H
#include<vector>
#include<unordered_map>
#include<unordered_set>
#include"Config/TextConfig.h"
#include"rapidjson/document.h"
namespace Sentry
{
    class NodeConfig
    {
    public:
        NodeConfig(const std::string & name)
            : mName(name), mIsAutoAllot(false) { }
    public:
        bool OnLoadConfig(const rapidjson::Value & value);
    public:
        bool IsAuthAllot() const { return this->mIsAutoAllot; }
        const std::string & GetName() const { return this->mName;}
        size_t GetComponents(std::vector<std::string> & components) const;
        size_t GetServices(std::vector<std::string> & services, bool start = false) const;
    private:
        bool mIsAutoAllot;
        const std::string & mName;
        std::unordered_set<std::string> mComponents;
        std::unordered_map<std::string, bool> mServices;
    };
}

namespace Sentry
{
    class ClusterConfig : public TextConfig
    {
    public:
        ClusterConfig() : TextConfig("ClusterConfig") { }

    private:
        bool OnLoadText(const std::string &content) final;
        bool OnReloadText(const std::string &content) final;
    public:
        const NodeConfig * GetConfig() const;
        const NodeConfig * GetConfig(const std::string & name) const;
    private:
        std::unordered_map<std::string, std::unique_ptr<NodeConfig>> mNodeConfigs;
    };
}


#endif //APP_CLUSTERCONFIG_H
