//
// Created by zmhy0073 on 2022/10/25.
//

#ifndef APP_LOCATIONSERVICE_H
#define APP_LOCATIONSERVICE_H
#include"Service/LocalRpcService.h"
namespace Sentry
{
    class LocationService : public LocalRpcService
    {
    public:
        LocationService() = default;
    public:
        bool OnStart() final;
        bool OnClose() final;
    private:
        XCode Add(const s2s::location::add & request);
        XCode Del(const s2s::location::del & request);
    private:
        class LocationComponent * mLocationComponent;
    };
}


#endif //APP_LOCATIONSERVICE_H
