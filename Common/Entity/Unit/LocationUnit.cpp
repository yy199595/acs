//
// Created by zmhy0073 on 2022/10/14.
//

#include"LocationUnit.h"
namespace Sentry
{
    LocationUnit::LocationUnit(long long id)
        : Unit(id) { }

    LocationUnit::LocationUnit(long long id, const std::string &address)
        : Unit(id), mAddress(address) { }

    bool LocationUnit::Del(const std::string &service)
    {
        auto iter = this->mLocations.find(service);
        if(iter == this->mLocations.end())
        {
            return false;
        }
        this->mLocations.erase(iter);
        return true;
    }
    void LocationUnit::Add(const std::string &server, const std::string &address)
    {
        this->mLocations[server] = address;
    }

    bool LocationUnit::Get(std::vector<std::string> &servers)
    {
        auto iter = this->mLocations.begin();
        for (; iter != this->mLocations.end(); iter++)
        {
            servers.emplace_back(iter->first);
        }
        return servers.size() > 0;
    }

    bool LocationUnit::Get(const std::string &server, std::string &address) const
    {
        auto iter = this->mLocations.find(server);
        if(iter == this->mLocations.end())
        {
            return false;
        }
        address = iter->second;       
        return true;
    }
}