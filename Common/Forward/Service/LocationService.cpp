//
// Created by zmhy0073 on 2022/10/25.
//

#include"LocationService.h"
#include"Component/LocationComponent.h"
namespace Sentry
{
    bool LocationService::OnStart()
    {
        BIND_COMMON_RPC_METHOD(LocationService::Add);
        BIND_COMMON_RPC_METHOD(LocationService::Del);
        this->mLocationComponent = this->GetComponent<LocationComponent>();
        return true;
    }

    bool LocationService::OnClose()
    {
        return true;
    }

    XCode LocationService::Add(const s2s::location::add &request)
    {
        long long userId = request.user_id();
        std::unique_ptr<LocationUnit> locationUnit(new LocationUnit(userId));
        for(auto & value : request.services())
        {
            locationUnit->Add(value.first, value.second);
        }
        this->mLocationComponent->AddLocationUnit(std::move(locationUnit));
        return XCode::Successful;
    }

    XCode LocationService::Del(const s2s::location::del &request)
    {
        long long userId = request.user_id();
        LocationUnit *locationUnit = this->mLocationComponent->GetLocationUnit(userId);
        if (locationUnit == nullptr)
        {
            return XCode::NotFindUser;
        }

        if (request.services_size() == 0)
        {
            this->mLocationComponent->DelLocationUnit(userId);
            return XCode::Successful;
        }

        for (const std::string &service: request.services())
        {
            locationUnit->Del(service);
        }
        return XCode::Successful;
    }
}