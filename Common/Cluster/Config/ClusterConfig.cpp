//
// Created by zmhy0073 on 2022/10/17.
//

#include"ClusterConfig.h"
#include"Core/System/System.h"
#include"Server/Config/ServerConfig.h"
namespace acs
{
    bool NodeConfig::OnLoadConfig(const json::r::Value &value)
    {
		this->mServices.clear();
		this->mComponents.clear();
		std::vector<std::string> services;
		std::vector<std::string> components;
		if(value.Get("Service", services))
		{
			this->mServices.insert(services.begin(), services.end());
		}
		if(value.Get("Component", components))
		{
			this->mComponents.insert(components.begin(), components.end());
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

	bool NodeConfig::HasService(const std::string& service) const
	{
		auto iter = this->mServices.find(service);
		return iter != this->mServices.end();
	}

}

namespace acs
{
    bool ClusterConfig::OnLoadJson()
    {
		std::vector<const char *> keys;
		if(this->GetKeys(keys) <= 0)
		{
			return false;
		}
		std::unique_ptr<json::r::Value> value;
		for(const char * key : keys)
		{
			if(!this->Get(key, value))
			{
				return false;
			}
			const std::string name(key);
			std::unique_ptr<NodeConfig> nodeConfig = std::make_unique<NodeConfig>(name);
			{
				nodeConfig->OnLoadConfig(*value);
				std::vector<std::string> services;
				nodeConfig->GetServices(services);
				for (const std::string& service: services)
				{
					this->mServiceNodes[service] = name;
				}
			}
			this->mNodeConfigs.emplace(name, std::move(nodeConfig));
		}
        return this->GetConfig() != nullptr;
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

    bool ClusterConfig::OnReLoadJson()
    {
        return true;
    }

    const NodeConfig *ClusterConfig::GetConfig() const
    {
		const std::string & name = ServerConfig::Inst()->Name();
        auto iter = this->mNodeConfigs.find(name);
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