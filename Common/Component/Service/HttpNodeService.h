//
// Created by yjz on 2022/1/26.
//

#ifndef SENTRY_HTTPNODESERVICE_H
#define SENTRY_HTTPNODESERVICE_H
#include"Component/Component.h"
#include"Component/Service/HttpService.h"
namespace Sentry
{
    class HttpNodeService : public HttpService
    {
    public:
        HttpNodeService() = default;
        ~HttpNodeService() final = default;
    public:
        XCode Push(const Json::Reader & request, Json::Writer & response);
    protected:
        bool Awake() final;
        bool LateAwake() final;
    private:
        class ServiceMgrComponent * mServiceComponent;
    };
}
#endif //SENTRY_HTTPNODESERVICE_H
