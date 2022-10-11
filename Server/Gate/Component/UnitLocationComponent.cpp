//
// Created by zmhy0073 on 2022/10/11.
//

#include"UnitLocationComponent.h"
#include"Component/OuterNetComponent.h"
#include"String/StringHelper.h"
namespace Sentry
{
    bool UnitLocationComponent::LateAwake()
    {
        this->mOuterComponent = this->GetApp()->GetComponent<OuterNetComponent>();
        return true;
    }

    bool UnitLocationComponent::Del(const std::string &name)
    {
        auto iter = this->mLocationMap.find(name);
        if(iter == this->mLocationMap.end())
        {
            return false;
        }
        this->mLocationMap.erase(iter);
        return true;
    }

    bool UnitLocationComponent::Get(const std::string &name, std::string &address)
    {
        auto iter = this->mLocationMap.find(name);
        if(iter == this->mLocationMap.end())
        {
            return false;
        }
        address = iter->second;
        return true;
    }

    bool UnitLocationComponent::Add(const std::string &addres)
    {
        this->mAddress = addres;
        return true;
    }

    bool UnitLocationComponent::Add(const std::string &name, const std::string &address)
    {
        auto iter = this->mLocationMap.find(name);
        if(iter != this->mLocationMap.end())
        {
            return false;
        }
        this->mLocationMap.emplace(name, address);
        return true;
    }

}