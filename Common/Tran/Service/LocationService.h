//
// Created by zmhy0073 on 2022/10/25.
//

#ifndef APP_LOCATIONSERVICE_H
#define APP_LOCATIONSERVICE_H
#include"Service/PhysicalService.h"
namespace Sentry
{
    class LocationService : public PhysicalService
    {
    public:
        LocationService() = default;
    public:
        bool OnStart() final;
        bool OnClose() final;
    private:
        XCode Add(const s2s::location::add & request);
        XCode Del(const s2s::location::del & request);
		XCode Register(const s2s::cluster::server & request, s2s::cluster::list & response);
    private:
        class LocationComponent * mLocationComponent;
    };
}


#endif //APP_LOCATIONSERVICE_H
