//
// Created by zmhy0073 on 2022/8/12.
//

#include"NodeMgrComponent.h"
#include"Util/Math/MathHelper.h"
#include"Util/Time/TimeHelper.h"
#include"Registry/Service/Registry.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Async/Component/AsyncMgrComponent.h"
#include"Entity/Unit/App.h"
#include"Util/String/StringHelper.h"
#include"Server/Config/CodeConfig.h"
namespace Tendo
{
    bool NodeMgrComponent::AddServer(int id, const std::string& server, const std::string & name, const std::string& address)
    {
		LOG_CHECK_RET_FALSE(!address.empty());
		LOG_CHECK_RET_FALSE(ClusterConfig::Inst()->GetConfig(server));

		auto iter = this->mServers.find(id);
		if(iter == this->mServers.end())
		{
			this->mServers.emplace(id, std::make_unique<LocationUnit>());
		}
		this->mServers[id]->Add(name, address);
		auto iter1 = this->mRpcServers.find(server);
		if(iter1 == this->mRpcServers.end())
		{
			std::vector<int> list;
			this->mRpcServers.emplace(server, list);
		}
		std::vector<int> &list = this->mRpcServers[server];
		if(std::find(list.begin(), list.end(), id) == list.end())
		{
			list.emplace_back(id);
		}
		LOG_INFO(server << " add " << name << " [" << address << "]");
		return true;
    }

	bool NodeMgrComponent::DelServer(int id)
	{
		auto iter = this->mServers.find(id);
		if (iter == this->mServers.end())
		{
			return false;
		}
		this->mServers.erase(iter);
		return true;
	}

    bool NodeMgrComponent::GetServer(const std::string & server, std::string & address, const char * listen)
	{
		std::string name("rpc");
		if(listen != nullptr)
		{
			name.assign(listen);
		}
		auto iter = this->mRpcServers.find(server);
		if (iter == this->mRpcServers.end() || iter->second.empty())
		{
			return false;
		}
		int size = (int)iter->second.size();
		int idx = Helper::Math::Random<int>(0, size - 1);
		auto iter1 = this->mServers.find(iter->second[idx]);
		if(iter1 == this->mServers.end())
		{
			return false;
		}
		return iter1->second->Get(name, address);
	}

    bool NodeMgrComponent::GetServer(const std::string & name, long long index, std::string & address)
    {
		auto iter1 = this->mClients.find(index);
		if (iter1 != this->mClients.end())
		{
			if (iter1->second->Get(name, address))
			{
				return true;
			}
		}

        auto iter = this->mRpcServers.find(name);
        if(iter == this->mRpcServers.end())
        {
            return false;
        }
        int size = (int)iter->second.size();
        address = iter->second[index % size];
        return true;
    }

	bool NodeMgrComponent::DelUnit(long long id)
	{
		auto iter = this->mClients.find(id);
		if(iter == this->mClients.end())
		{
			return false;
		}
		this->mClients.erase(iter);
		return true;
	}


	void NodeMgrComponent::AddRpcServer(const std::string& server, long long userId, const std::string& address)
	{
		auto iter = this->mClients.find(userId);
		if (iter == this->mClients.end())
		{
			std::unique_ptr<LocationUnit> tmp = std::make_unique<LocationUnit>();
			{
				tmp->Add(server, address);
				this->mClients.emplace(userId, std::move(tmp));
			}
			return;
		}
		iter->second->Add(server, address);
	}

    bool NodeMgrComponent::GetServer(long long int userId, std::unordered_map<std::string, std::string> &servers)
    {
        auto iter = this->mClients.find(userId);
        if(iter == this->mClients.end())
        {
            return false;
        }
		return iter->second->Get(servers);
    }

	bool NodeMgrComponent::GetServer(const string& server, std::vector<std::string>& servers, const char * listen)
	{
		std::string name("rpc");
		if(listen != nullptr)
		{
			name.assign(listen);
		}
		auto iter = this->mRpcServers.find(server);
		if(iter == this->mRpcServers.end())
		{
			return false;
		}
		for (auto& id : iter->second)
		{
			auto iter1 = this->mServers.find(id);
			if(iter1 != this->mServers.end())
			{
				std::string address;
				if (iter1->second->Get(name, address))
				{
					servers.emplace_back(address);
				}
			}
		}
		return !servers.empty();
	}

	bool NodeMgrComponent::HasServer(const string& server) const
	{
		auto iter = this->mRpcServers.find(server);
		if(iter == this->mRpcServers.end())
		{
			return false;
		}
		return iter->second.size() > 0;
	}
}