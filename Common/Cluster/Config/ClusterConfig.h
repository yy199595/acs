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
#include"Singleton/Singleton.h"
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
        bool IsStart(const std::string & service) const;
        bool HasService(const std::string & service) const;
        bool IsAuthAllot() const { return this->mIsAutoAllot; }
        const std::string & GetName() const { return this->mName;}
        const std::string & GetMainLua() const { return this->mLua ;}
        size_t ServiceCount() const { return this->mServices.size(); }
        size_t GetComponents(std::vector<std::string> & components) const;
        size_t GetServices(std::vector<std::string> & services, bool start = false) const;
        void GetWaitServer(std::unordered_set<std::string>& list) const;
    private:
		std::string mLua;
        bool mIsAutoAllot;
        const std::string mName;
        std::unordered_set<std::string> mComponents;
        std::unordered_map<std::string, bool> mServices;
    };
}

namespace Sentry
{
    class ClusterConfig : public TextConfig, public ConstSingleton<ClusterConfig>
    {
    public:
        ClusterConfig() : TextConfig("ClusterConfig") { }

    private:
        bool OnLoadText(const char *str, size_t length) final;
        bool OnReloadText(const char *str, size_t length) final;
    public:
        const NodeConfig * GetConfig() const;
        const NodeConfig * GetConfig(const std::string & name) const;
        size_t GetNodeConfigs(std::vector<const NodeConfig *> & nodes) const;
        bool GetServerName(const std::string& service, std::string& node) const;
    private:
        std::unordered_map<std::string, std::string> mServiceNodes;
        std::unordered_map<std::string, std::unique_ptr<NodeConfig>> mNodeConfigs;
    };
}


#endif //APP_CLUSTERCONFIG_H
