//
// Created by zmhy0073 on 2022/8/12.
//

#include"LocationComponent.h"
#include"Log/CommonLogDef.h"


namespace Sentry
{
	void LocationComponent::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
		luaRegister.BeginRegister<LocationComponent>();

	}
}

namespace Sentry
{
    void LocationComponent::AddLocation(const std::string& service, const std::string& address)
    {
		auto iter = this->mServiceLocations.find(service);
		if(iter == this->mServiceLocations.end())
		{
			std::vector<HostCounter> item;
			this->mServiceLocations.emplace(service, item);
		}
		HostCounter hostCounter(address);
		this->mServiceLocations[service].emplace_back(hostCounter);
    }

    bool LocationComponent::DelLocation(const std::string& address)
    {

        return true;
    }

	bool LocationComponent::DelLocation(const std::string& service, const std::string& address)
	{
		return false;
	}

	bool LocationComponent::GetLocationss(const std::string& service, std::vector<std::string>& hosts)
	{
		auto iter = this->mServiceLocations.find(service);
		if(iter == this->mServiceLocations.end())
		{
			return false;
		}
		for(const HostCounter & hostCounter : iter->second)
		{
			hosts.emplace_back(hostCounter.Address);
		}
		return true;
	}

	size_t LocationComponent::GetHostSize(const std::string& service) const
	{
		auto iter = this->mServiceLocations.find(service);
		return iter != this->mServiceLocations.end() ? iter->second.size() : 0;
	}

	LocationUnit* LocationComponent::GetLocationUnit(long long id) const
	{
		auto iter = this->mUnitLocations.find(id);
		return iter != this->mUnitLocations.end() ? iter->second.get() : nullptr;
	}

	bool LocationComponent::HasLocation(const string& service, const string& address)
	{
		auto iter = this->mServiceLocations.find(service);
		if(iter == this->mServiceLocations.end())
		{
			return false;
		}
		for(const HostCounter & hostCounter : iter->second)
		{
			if(hostCounter.Address == address)
			{
				return true;
			}
		}
		return false;
	}

	bool LocationComponent::AllotLocation(const string& service, string& address)
	{
		auto iter = this->mServiceLocations.find(service);
		if(iter == this->mServiceLocations.end())
		{
			return false;
		}
		HostCounter * returnHost = &iter->second.at(0);
		for(HostCounter & hostCounter : iter->second)
		{
			if(hostCounter.Count <= 100)
			{
				hostCounter.Count++;
				address = hostCounter.Address;
				return true;
			}
			if(hostCounter.Count < returnHost->Count)
			{
				returnHost = &hostCounter;
			}
		}
		returnHost->Count++;
		address = returnHost->Address;
		return true;
	}
	bool LocationComponent::DelLocationUnit(long long int id)
	{
		auto iter = this->mUnitLocations.find(id);
		if(iter == this->mUnitLocations.end())
		{
			return false;
		}
		this->mUnitLocations.erase(iter);
		return true;
	}

	LocationUnit* LocationComponent::AddLocationUnit(long long id)
	{
		auto iter = this->mUnitLocations.find(id);
		if(iter != this->mUnitLocations.end())
		{
			return iter->second.get();
		}
		std::unique_ptr<LocationUnit> locationUnit(new LocationUnit(id));
		this->mUnitLocations.emplace(id, std::move(locationUnit));
		return this->GetLocationUnit(id);
	}

	LocationUnit* LocationComponent::AddLocationUnit(long long id, const std::string& address)
	{
		auto iter = this->mUnitLocations.find(id);
		if(iter != this->mUnitLocations.end())
		{
			return iter->second.get();
		}
		std::unique_ptr<LocationUnit> locationUnit(new LocationUnit(id, address));
		this->mUnitLocations.emplace(id, std::move(locationUnit));
		return this->GetLocationUnit(id);
	}

}