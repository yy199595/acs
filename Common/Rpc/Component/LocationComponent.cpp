//
// Created by zmhy0073 on 2022/8/12.
//

#include"LocationComponent.h"
#include"Util/Math/MathHelper.h"
#include"Util/Time/TimeHelper.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Async/Component/CoroutineComponent.h"
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
	bool LocationComponent::DelServer(int id, const std::string& name)
	{
		auto iter = this->mServers.find(id);
		if (iter == this->mServers.end())
		{
			return false;
		}
		this->mServers.erase(iter);
		auto iter1 = this->mServerNames.find(name);
		if(iter1 != this->mServerNames.end())
		{
			std::remove(iter1->second.begin(), iter1->second.end(), id);
		}
		return true;
	}

    bool LocationComponent::GetServer(const std::string & server, std::string & address, const std::string & listen)
	{
		auto iter = this->mServerNames.find(server);
		if (iter == this->mServerNames.end() || iter->second.empty())
		{
			return false;
		}
		int size = (int)iter->second.size();
		int id = iter->second[Helper::Math::Random<int>(0, size - 1)];
		auto iter1 = this->mServers.find(id);
		if(iter1 == this->mServers.end())
		{
			return false;
		}
		return iter1->second->Get(listen, address);
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

	bool LocationComponent::HasServer(const std::string& server) const
	{
		auto iter = this->mServerNames.find(server);
		if(iter == this->mServerNames.end())
		{
			return false;
		}
		return iter->second.size() > 0;
	}

	LocationUnit* LocationComponent::GetOrCreateServer(int id, const std::string& name)
	{
		LocationUnit* ret = nullptr;
		auto iter = this->mServers.find(id);
		if (iter != this->mServers.end())
		{
			ret = iter->second.get();
		}
		else
		{
			std::unique_ptr<LocationUnit> locationUnit
					= std::make_unique<LocationUnit>(name, id);
			ret = locationUnit.get();
			this->mServers.emplace(id, std::move(locationUnit));
			auto iter = this->mServerNames.find(name);
			if(iter == this->mServerNames.end())
			{
				std::vector<int> servers;
				this->mServerNames.emplace(name, servers);
			}
			this->mServerNames[name].emplace_back(id);
		}
		return ret;
	}

	bool LocationComponent::GetServer(const std::string& server, std::vector<std::string>& servers, const std::string& listen)
	{
		auto iter = this->mServerNames.find(server);
		if(iter == this->mServerNames.end())
		{
			return false;
		}
		for(int id : iter->second)
		{
			auto iter1 = this->mServers.find(id);
			if(iter1 != this->mServers.end())
			{
				std::string address;
				iter1->second->Get(listen, address);
				servers.emplace_back(address);
			}
		}
		return !servers.empty();
	}

	void LocationComponent::GetAllServer(std::vector<LocationUnit*>& servers)
	{
		servers.reserve(this->mServers.size());
		auto iter = this->mServers.begin();
		for(; iter != this->mServers.end(); iter++)
		{
			servers.emplace_back(iter->second.get());
		}
		std::sort(servers.begin(), servers.end(), [](LocationUnit * p1, LocationUnit * p2)->bool {
			return p1->GetId() > p2->GetId();
		});
	}
}