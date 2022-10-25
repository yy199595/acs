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
    void LocationUnit::Add(const std::string &service, const std::string &address)
    {

        this->mLocations[service] = address;
    }

    bool LocationUnit::Get(std::vector<std::string> &services)
    {
        auto iter = this->mLocations.begin();
        for (; iter != this->mLocations.end(); iter++)
        {
            services.emplace_back(iter->first);
        }
        return services.size() > 0;
    }

    bool LocationUnit::Get(const std::string &service, std::string &address) const
    {
        auto iter = this->mLocations.find(service);
        if(iter == this->mLocations.end())
        {
            return false;
        }
        address = iter->second;
        return true;
    }
}