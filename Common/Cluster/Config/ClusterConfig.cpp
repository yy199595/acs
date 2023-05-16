//
// Created by zmhy0073 on 2022/10/17.
//

#include"ClusterConfig.h"
#include"Core/System/System.h"
#include"Server/Config/ServerConfig.h"
namespace Tendo
{
    bool NodeConfig::OnLoadConfig(const rapidjson::Value &value, int index)
    {
		this->mIndex = index;
		this->mIsAutoAllot = false;
        if(value.HasMember("Service"))
        {
            if(!value["Service"].IsArray())
            {
                return false;
            }
			const rapidjson::Value & jsonArray = value["Service"];
			for(unsigned int index = 0; index<jsonArray.Size();index++)
			{
				const rapidjson::Value& element = jsonArray[index];
				this->mServices.emplace(std::string(element.GetString()));
			}
        }
        if(value.HasMember("Component"))
        {
            if(!value["Component"].IsArray())
            {
                return false;
            }
            for(size_t index = 0; index < value["Component"].Size(); index++)
            {
                const rapidjson::Value & data = value["Component"][index];
                this->mComponents.emplace(std::string(data.GetString()));
            }
        }
        if(value.HasMember("AutoAllot"))
        {
            this->mIsAutoAllot = value["AutoAllot"].GetBool();
        }
        if(value.HasMember("lua"))
        {
            this->mLua = value["lua"].GetString();
        }
        return true;
    }

    size_t NodeConfig::GetComponents(std::vector<std::string> &components) const
    {
        for(const std::string & name : this->mComponents)
        {
            components.emplace_back(name);
        }
        return components.size();
    }

    size_t NodeConfig::GetServices(std::vector<std::string> &services) const
    {
        for(const std::string & value : this->mServices)
        {
            services.emplace_back(value);
        }
        return services.size();
    }

}

namespace Tendo
{
    bool ClusterConfig::OnLoadText(const char *str, size_t length)
    {
        rapidjson::Document document;
        if(document.Parse(str, length).HasParseError())
        {
            return false;
        }
		int index = 0;
        auto iter = document.MemberBegin();
        for(; iter != document.MemberEnd(); iter++)
        {
            const std::string name(iter->name.GetString());
            const rapidjson::Value & jsonValue = iter->value;
            std::unique_ptr<NodeConfig> nodeConfig(new NodeConfig(name));
            if(!nodeConfig->OnLoadConfig(jsonValue, index++))
            {
                return false;
            }
            std::vector<std::string> services;
            nodeConfig->GetServices(services);
            for (const std::string& service : services)
            {               
                this->mServiceNodes[service] = name;
            }
            this->mNodeConfigs.emplace(name, std::move(nodeConfig));
        }
        return true;
    }

    bool ClusterConfig::GetServerName(const std::string& service, std::string& node) const
    {
        auto iter = this->mServiceNodes.find(service);
        if (iter == this->mServiceNodes.end())
        {
            return false;
        }
        node = iter->second;
        return true;
    }

    bool ClusterConfig::OnReloadText(const char *str, size_t length)
    {
        return true;
    }

    const NodeConfig *ClusterConfig::GetConfig() const
    {        
        auto iter = this->mNodeConfigs.find(ServerConfig::Inst()->Name());
        return iter != this->mNodeConfigs.end() ? iter->second.get() : nullptr;
    }

    const NodeConfig *ClusterConfig::GetConfig(const std::string &name) const
    {
        auto iter = this->mNodeConfigs.find(name);
        return iter != this->mNodeConfigs.end() ? iter->second.get() : nullptr;
    }

    void ClusterConfig::GetServers(std::vector<std::string>& list) const
    {
        auto iter = this->mNodeConfigs.begin();
        for (; iter != this->mNodeConfigs.end(); iter++)
        {
            if (iter->second->ServiceCount() > 0)
            {
                list.emplace_back(iter->second->GetName());
            }
        }
    }

    size_t ClusterConfig::GetNodeConfigs(std::vector<const NodeConfig *> &nodes) const
    {
        auto iter = this->mNodeConfigs.begin();
        for(; iter != this->mNodeConfigs.end(); iter++)
        {
            nodes.emplace_back(iter->second.get());
        }
        return nodes.size();
    }
}