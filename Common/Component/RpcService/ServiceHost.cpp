//
// Created by zmhy0073 on 2022/8/12.
//

#include"ServiceHost.h"
#include"Define/CommonLogDef.h"
#include"App/App.h"
namespace Sentry
{
    void ServiceHost::AddHost(const std::string &address)
    {
        if(!this->HasHost(address))
        {
            this->OnAddHost(address);
            const std::string & name = this->GetServiceName();
            this->mHosts.emplace_back(new HostCounter(address));
            CONSOLE_LOG_ERROR(name << " add host [" << address <<"]");
        }
    }


    void ServiceHost::AddHost(const std::string &address, long long unitId)
    {
        this->mUnitHosts[unitId] = address;
        if(address == this->GetLocalHost())
        {
            this->OnUnitJoin(unitId);
        }
    }

    bool ServiceHost::DelHost(long long unitId)
    {
        auto iter = this->mUnitHosts.find(unitId);
        if(iter != this->mUnitHosts.end())
        {
            if(iter->second == this->GetLocalHost())
            {
                this->OnUnitExit(unitId);
            }
            this->mUnitHosts.erase(iter);
            return true;
        }
        return false;
    }

    bool ServiceHost::DelHost(const std::string &address)
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

    bool ServiceHost::GetHost(std::string &address, int max)
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

    bool ServiceHost::HasHost(const std::string &address)
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

    bool ServiceHost::GetHosts(std::vector<std::string> &hosts)
    {
        for(HostCounter * counter : this->mHosts)
        {
            hosts.emplace_back(counter->Address);
        }
        return !hosts.empty();
    }

    bool ServiceHost::GetHost(long long unitId, std::string &address)
    {
        auto iter = this->mUnitHosts.find(unitId);
        if(iter != this->mUnitHosts.end())
        {
            address = iter->second;
            return true;
        }
        return false;
    }
}