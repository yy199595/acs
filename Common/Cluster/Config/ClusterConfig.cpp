//
// Created by zmhy0073 on 2022/10/17.
//

#include<algorithm>
#include"ClusterConfig.h"
#include"Core/System/System.h"
#include"Server/Config/ServerConfig.h"
namespace acs
{
    bool NodeConfig::OnLoadConfig(const json::r::Value &value)
    {
		this->mComponents.clear();
		this->mRpcServices.clear();
		this->mHttpServices.clear();
		std::vector<std::string> services;
		std::vector<std::string> components;
		value.Get("rpc", this->mRpcServices);
		value.Get("http", this->mHttpServices);
		value.Get("component", this->mComponents);
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

    size_t NodeConfig::GetRpcServices(std::vector<std::string> &services) const
    {
		services.insert(services.end(), this->mRpcServices.begin(), this->mRpcServices.end());
		return services.size();
    }

	size_t NodeConfig::GetHttpServices(std::vector<std::string>& services) const
	{
		services.insert(services.end(), this->mHttpServices.begin(), this->mHttpServices.end());
		return services.size();
	}

	bool NodeConfig::HasService(const std::string& service) const
	{
		auto iter = std::find(this->mRpcServices.begin(), this->mRpcServices.end(), service);
		if(iter == this->mRpcServices.end())
		{
			iter = std::find(this->mHttpServices.begin(), this->mHttpServices.end(), service);
			if(iter == this->mHttpServices.end())
			{
				return false;
			}
		}
		return true;
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
				nodeConfig->GetRpcServices(services);
				nodeConfig->GetHttpServices(services);
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