//
// Created by zmhy0073 on 2022/10/17.
//

#ifndef APP_CLUSTERCONFIG_H
#define APP_CLUSTERCONFIG_H
#include<vector>
#include<unordered_map>
#include<unordered_set>
#include"rapidjson/document.h"
#include"Config/Base/JsonConfig.h"
#include"Core/Singleton/Singleton.h"
namespace Tendo
{
    class NodeConfig
    {
    public:
        NodeConfig(const std::string & name)
            : mIsAutoAllot(false), mName(name) { }
    public:
        bool OnLoadConfig(const rapidjson::Value & value, int index);
    public:
		int GetIndex() const { return this->mIndex;}
        bool IsAuthAllot() const { return this->mIsAutoAllot; }
        const std::string & GetName() const { return this->mName;}
        const std::string & GetMainLua() const { return this->mLua ;}
        size_t ServiceCount() const { return this->mServices.size(); }
		size_t GetServices(std::vector<std::string> & services) const;
		size_t GetComponents(std::vector<std::string> & components) const;
    private:
		int mIndex; //决定了服务器关闭顺序
		std::string mLua;
        bool mIsAutoAllot;
        const std::string mName;
		std::unordered_set<std::string> mServices;
		std::unordered_set<std::string> mComponents;
    };
}

namespace Tendo
{
    class ClusterConfig : public JsonConfig, public ConstSingleton<ClusterConfig>
    {
    public:
        ClusterConfig() : JsonConfig("ClusterConfig") { }

    private:
		bool OnLoadJson(rapidjson::Document &document) final;
		bool OnReLoadJson(rapidjson::Document &document) final;
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
