//
// Created by zmhy0073 on 2022/10/8.
//

#ifndef APP_INNERSERVICE_H
#define APP_INNERSERVICE_H
#include"LocalService.h"
namespace Sentry
{
    class InnerService : public LocalService
    {
    public:
        InnerService() = default;
        ~InnerService() = default;

    private:
        XCode Ping();
        XCode Hotfix();
        XCode StartService(const com::type::string & request);
        XCode CloseService(const com::type::string & request);
    private:
        void Awake();
        bool OnStart() final;
        bool OnClose() final { return false; }
    };
}


#endif //APP_INNERSERVICE_H
