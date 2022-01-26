//
// Created by yjz on 2022/1/26.
//

#ifndef SENTRY_HTTPNODESERVICE_H
#define SENTRY_HTTPNODESERVICE_H
#include"Component.h"
#include"Service/HttpService.h"
namespace Sentry
{
    class HttpNodeService : public HttpService
    {
    public:
        HttpNodeService() = default;
        ~HttpNodeService() final = default;
    public:
        XCode Push(const RapidJsonReader & request, RapidJsonWriter & response);
    protected:
        bool Awake() final;
        bool LateAwake() final;
    private:
        class ServiceProxyComponent * mServiceComponent;
    };
}
#endif //SENTRY_HTTPNODESERVICE_H
