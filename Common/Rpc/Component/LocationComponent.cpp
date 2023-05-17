//
// Created by zmhy0073 on 2022/8/12.
//

#include"LocationComponent.h"
#include"Util/Math/MathHelper.h"
#include"Util/Time/TimeHelper.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Async/Component/CoroutineComponent.h"
#include"Entity/Actor/App.h"
#include"Util/String/StringHelper.h"
#include"Server/Config/CodeConfig.h"
#include"Sqlite/Component/SqliteComponent.h"

namespace Tendo
{
	bool LocationComponent::DelServer(int id)
	{
		ServerUnit * serverUnit = this->GetServerById(id);
		if(serverUnit == nullptr)
		{
			return false;
		}
		const std::string & name = serverUnit->Name();
		auto iter1 = this->mServerNames.find(name);
		if(iter1 != this->mServerNames.end())
		{
			std::remove(iter1->second.begin(), iter1->second.end(), id);
		}
		auto iter = this->mServers.find(id);
		if (iter != this->mServers.end())
		{
			this->mServers.erase(iter);
		}
		return true;
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

	ServerUnit* LocationComponent::GetOrCreateServer(int id, const std::string& name)
	{
		ServerUnit* ret = nullptr;
		auto iter = this->mServers.find(id);
		if (iter != this->mServers.end())
		{
			ret = iter->second.get();
		}
		else
		{
			std::unique_ptr<ServerUnit> locationUnit
					= std::make_unique<ServerUnit>(name, id);
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

	void LocationComponent::GetAllServer(std::vector<ServerUnit*>& servers)
	{
		servers.reserve(this->mServers.size());
		auto iter = this->mServers.begin();
		for(; iter != this->mServers.end(); iter++)
		{
			servers.emplace_back(iter->second.get());
		}
		std::sort(servers.begin(), servers.end(), [](ServerUnit * p1, ServerUnit * p2)->bool {
			return p1->GetId() > p2->GetId();
		});
	}

	int LocationComponent::RangeServer(const std::string& server) const
	{
		auto iter = this->mServerNames.find(server);
		if (iter == this->mServerNames.end() || iter->second.empty())
		{
			return -1;
		}
		int size = (int)iter->second.size();
		return iter->second[Helper::Math::Random<int>(0, size - 1)];
	}

	ServerUnit* LocationComponent::GetServerById(int id)
	{
		auto iter = this->mServers.find(id);
		if(iter == this->mServers.end())
		{
			return nullptr;
		}
		return iter->second.get();
	}

	bool LocationComponent::GetServerAddress(int id, const std::string& listen, std::string& address)
	{
		ServerUnit * serverUnit = this->GetServerById(id);
		if(serverUnit == nullptr)
		{
			return false;
		}
		return serverUnit->Get(listen, address);
	}
}