//
// Created by zmhy0073 on 2022/8/12.
//

#include"LocationComponent.h"
#include"Util/Math/MathHelper.h"
#include"Util/Time/TimeHelper.h"
#include"Registry/Service/Registry.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Async/Component/AsyncMgrComponent.h"
#include"Entity/Unit/App.h"
#include"Util/String/StringHelper.h"
#include"Server/Config/CodeConfig.h"
#include"Sqlite/Component/SqliteComponent.h"
namespace Tendo
{
	ServerData::ServerData(const std::string& name)
		: mName(name)
	{

	}

	bool ServerData::Get(const std::string& listen, std::string & address) const
	{
		auto iter = this->mListens.find(listen);
		if(iter == this->mListens.end())
		{
			return false;
		}
		address = iter->second;
		return true;
	}

	bool ServerData::Add(const std::string& listen, const std::string& address)
	{
		auto iter = this->mListens.find(listen);
		if(iter == this->mListens.end())
		{
			if(listen == "rpc")
			{
				this->mRpc = address;
			}
			this->mListens.emplace(listen, address);
			return true;
		}
		return false;
	}


}


namespace Tendo
{
    bool LocationComponent::AddServer(const ServerData& serverData)
    {
		const std::string & name = serverData.Name();
		const std::string & rpc = serverData.RpcAddress();
		auto iter = this->mServers.find(name);
		if(iter == this->mServers.end())
		{
			std::vector<ServerData> list;
			this->mServers.emplace(name, list);
		}

		std::vector<ServerData> & list = this->mServers[name];
		auto iter1 = std::find(list.begin(), list.end(), rpc);
		if(iter1 != list.end())
		{
			return false;
		}
		list.emplace_back(serverData);
		return true;
    }

	bool LocationComponent::DelServer(const std::string & server, const std::string & rpc)
	{
		auto iter = this->mServers.find(server);
		if (iter == this->mServers.end())
		{
			return false;
		}
		std::vector<ServerData>& list = iter->second;
		list.erase(std::remove_if(list.begin(), list.end(), [&](const ServerData& info)
		{
			return info.RpcAddress() == rpc;
		}), list.end());
		return true;
	}

    bool LocationComponent::GetServer(const std::string & server, std::string & address, const std::string & listen)
	{
		auto iter = this->mServers.find(server);
		if (iter == this->mServers.end() || iter->second.empty())
		{
			return false;
		}
		int size = (int)iter->second.size();
		int idx = Helper::Math::Random<int>(0, size - 1);
		return iter->second[idx].Get(listen, address);
	}

    bool LocationComponent::GetServer(const std::string & name, long long userId, std::string & address)
    {
		auto iter1 = this->mClients.find(userId);
		if (iter1 == this->mClients.end())
		{
			return false;
		}
		return iter1->second->Get(name, address);
    }

	bool LocationComponent::DelUnit(long long userId)
	{
		auto iter = this->mClients.find(userId);
		if(iter == this->mClients.end())
		{
			return false;
		}
		this->mClients.erase(iter);
		return true;
	}

	bool LocationComponent::DelUnit(const std::string& server, long long userId)
	{
		auto iter = this->mClients.find(userId);
		if(iter == this->mClients.end())
		{
			return false;
		}
		return iter->second->Del(server);
	}


	void LocationComponent::BindServer(const std::string& server, long long userId, const std::string& address)
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

    bool LocationComponent::GetServer(long long int userId, std::unordered_map<std::string, std::string> &servers)
    {
        auto iter = this->mClients.find(userId);
        if(iter == this->mClients.end())
        {
            return false;
        }
		return iter->second->Get(servers);
    }

	bool LocationComponent::GetServer(const string& server, std::vector<std::string>& servers, const std::string & listen)
	{
		auto iter = this->mServers.find(server);
		if(iter == this->mServers.end())
		{
			return false;
		}
		std::string address;
		for (const ServerData & data : iter->second)
		{
			if(data.Get(listen, address))
			{
				servers.emplace_back(address);
			}
		}
		return !servers.empty();
	}

	bool LocationComponent::HasServer(const string& server) const
	{
		auto iter = this->mServers.find(server);
		if(iter == this->mServers.end())
		{
			return false;
		}
		return iter->second.size() > 0;
	}
}