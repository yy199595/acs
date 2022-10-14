//
// Created by zmhy0073 on 2022/10/14.
//

#include "LocationUnit.h"

namespace Sentry
{
    LocationUnit::LocationUnit(long long id)
        : mUnitId(id) { }

    LocationUnit::LocationUnit(long long id, const std::string &address)
        : mUnitId(id), mAddress(address) { }

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

    bool LocationUnit::Get(const std::string &service, std::string &address)
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