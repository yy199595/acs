//
// Created by zmhy0073 on 2022/8/12.
//

#include"LocationComponent.h"
#include"Log/CommonLogDef.h"
namespace Sentry
{
    void LocationComponent::AddLocation(const std::string &address)
    {
        if(!this->HasLocation(address))
        {
            this->OnAddHost(address);
            const std::string & name = this->GetServiceName();
            this->mHosts.emplace_back(new HostCounter(address));
            CONSOLE_LOG_DEBUG(name << " add host [" << address <<"]");
        }
    }

    bool LocationComponent::AddLocation(long long userId, const std::string &address)
    {
        if(!this->HasLocation(address))
        {
            this->mUnitLocations[userId] = address;
            return true;
        }
        return false;
    }

    bool LocationComponent::GetLocation(long long userId, std::string &address)
    {
        auto iter = this->mUnitLocations.find(userId);
        if(iter == this->mUnitLocations.end())
        {
            return false;
        }
        address = iter->second;
        return true;
    }

    bool LocationComponent::DelLocation(const std::string &address)
    {
        auto iter = this->mHosts.begin();
        for(; iter != this->mHosts.end(); iter++)
        {
            if((*iter)->Address == address)
            {
                this->OnDelHost(address);
                HostCounter * counter = (*iter);
                this->mHosts.erase(iter);
                delete counter;
                return true;
            }
        }
        return false;
    }

    bool LocationComponent::GetLocation(std::string &address, int max)
    {
        if(this->mHosts.empty())
        {
            return false;
        }
        HostCounter * hostCounter = this->mHosts.front();
        for(HostCounter * counter : this->mHosts)
        {
            if(counter->Count < max)
            {
                counter->Count++;
                address = counter->Address;
                return true;
            }
            if(hostCounter->Count < counter->Count)
            {
                hostCounter = counter;
            }
        }
        address = hostCounter->Address;
        return true;
    }

    bool LocationComponent::HasLocation(const std::string &address)
    {
        for(HostCounter * hostCounter : this->mHosts)
        {
            if (hostCounter->Address == address)
            {
                return true;
            }
        }
        return false;
    }

    bool LocationComponent::GetHosts(std::vector<std::string> &hosts)
    {
        for(HostCounter * counter : this->mHosts)
        {
            hosts.emplace_back(counter->Address);
        }
        return !hosts.empty();
    }
}