//
// Created by zmhy0073 on 2022/10/17.
//

#ifndef APP_CLUSTERCONFIG_H
#define APP_CLUSTERCONFIG_H
#include<vector>
#include<unordered_map>
#include<unordered_set>
#include"Config/Base/JsonConfig.h"
#include"Yyjson/Document/Document.h"
#include"Core/Singleton/Singleton.h"
namespace acs
{
    class NodeConfig
    {
    public:
        explicit NodeConfig(std::string  name) : mName(std::move(name)) { }
    public:
        bool OnLoadConfig(const json::r::Value & value);
    public:
		bool HasService(const std::string & service) const;
        const std::string & GetName() const { return this->mName;}
		size_t GetRpcServices(std::vector<std::string> & services) const;
		size_t GetHttpServices(std::vector<std::string> & services) const;
		size_t GetComponents(std::vector<std::string> & components) const;
		size_t ServiceCount() const { return this->mRpcServices.size() + this->mHttpServices.size(); }
    private:
        const std::string mName;
		std::vector<std::string> mComponents;
		std::vector<std::string> mRpcServices;
		std::vector<std::string> mHttpServices;
    };
}

namespace acs
{
    class ClusterConfig : public JsonConfig, public ConstSingleton<ClusterConfig>
    {
    public:
        ClusterConfig() : JsonConfig("ClusterConfig") { }

    private:
		bool OnLoadJson() final;
		bool OnReLoadJson() final;
    public:
        const NodeConfig * GetConfig() const;
        const NodeConfig * GetConfig(const std::string & name) const;
        void GetServers(std::vector<std::string>& list) const;
        size_t GetNodeConfigs(std::vector<const NodeConfig *> & nodes) const;
        bool GetServerName(const std::string& service, std::string& node) const;
    private:
        std::unordered_map<std::string, std::string> mServiceNodes;
        std::unordered_map<std::string, std::unique_ptr<NodeConfig>> mNodeConfigs;
    };
}


#endif //APP_CLUSTERCONFIG_H
