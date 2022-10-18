//
// Created by zmhy0073 on 2022/10/8.
//

#ifndef APP_INNERSERVICE_H
#define APP_INNERSERVICE_H
#include"LocalRpcService.h"
namespace Sentry
{
    class InnerService : public LocalRpcService
    {
    public:
        InnerService() = default;
        ~InnerService() = default;
    private:
        XCode Ping();
        XCode Stop();
        XCode Hotfix();
        XCode LoadConfig();
    private:
        bool Awake();
        bool OnStart() final;
        bool OnClose() final { return false; }

    private:
    };
}


#endif //APP_INNERSERVICE_H
