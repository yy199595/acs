//
// Created by zmhy0073 on 2022/8/12.
//

#include"LocationComponent.h"
#include"Log/CommonLogDef.h"

namespace Sentry
{
    void LocationComponent::AddRpcServer(const std::string& name, const std::string& address)
    {
        if(address.empty())
        {
            return;
        }
		auto iter = this->mRpcServers.find(name);
		if(iter == this->mRpcServers.end())
		{
			std::vector<std::string> item;
			this->mRpcServers.emplace(name, item);
		}
		this->mAllotCount.emplace(address, 0);
        std::vector<std::string> & locations = this->mRpcServers[name];
        if(std::find(locations.begin(), locations.end(), address) == locations.end())
        {
            locations.emplace_back(address);
            LOG_WARN(name << " add rpc server address [" << address << "]");
        }
    }

	void LocationComponent::AddHttpServer(const std::string& name, const std::string& address)
	{
		LOG_WARN(name << " add http server address [" << address << "]");
	}

	size_t LocationComponent::GetServerCount(const std::string & name)
	{
		auto iter = this->mRpcServers.find(name);
		if (iter == this->mRpcServers.end())
		{
			return 0;
		}
		return iter->second.size();

	}

	bool LocationComponent::DelServer(const std::string& address)
	{
		auto iter = this->mRpcServers.begin();
		for (; iter != this->mRpcServers.end(); iter++)
		{
			auto iter1 = std::find(iter->second.begin(), iter->second.end(), address);
			if (iter1 != iter->second.end())
			{
				iter->second.erase(iter1);
				return true;
			}
		}		
		return false;
	}

	bool LocationComponent::GetServers(const std::string& server, std::vector<std::string>& hosts)
	{
		auto iter = this->mRpcServers.find(server);
		if(iter == this->mRpcServers.end())
		{
			return false;
		}
		hosts = iter->second;
		return true;
	}

	LocationUnit* LocationComponent::GetUnit(long long id) const
	{
		auto iter = this->mUnitLocations.find(id);
		return iter != this->mUnitLocations.end() ? iter->second.get() : nullptr;
	}

	int LocationComponent::GetAllotCount(const std::string& address) const
	{
		auto iter = this->mAllotCount.find(address);
		return iter != this->mAllotCount.end() ? iter->second : 0;
	}

	bool LocationComponent::AllotServer(const string& server, string& address)
	{
		auto iter = this->mRpcServers.find(server);
		if(iter == this->mRpcServers.end() || iter->second.empty())
		{
			return false;
		}
		address = iter->second.at(0);
		int count = this->GetAllotCount(address);
		for(const std::string & location : iter->second)
		{
			int num = this->GetAllotCount(location);
			if(num <= 100)
			{
				address = location;
				this->mAllotCount[address]++;
				return true;
			}
			if(num < count)
			{
				address = location;
			}
		}
		this->mAllotCount[address]++;
		return true;
	}

	bool LocationComponent::DelUnit(long long id)
	{
		auto iter = this->mUnitLocations.find(id);
		if(iter == this->mUnitLocations.end())
		{
			return false;
		}
		this->mUnitLocations.erase(iter);
		return true;
	}

	bool LocationComponent::AddUnit(std::unique_ptr<LocationUnit> locationUnit)
	{
		if(locationUnit == nullptr)
		{
			return false;
		}
		long long id = locationUnit->GetUnitId();
		auto iter = this->mUnitLocations.find(id);
		if(iter != this->mUnitLocations.end())
		{
			return iter->second.get();
		}
		this->mUnitLocations.emplace(id, std::move(locationUnit));
		return true;
	}

}