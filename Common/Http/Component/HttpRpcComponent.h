//
// Created by zmhy0073 on 2022/8/11.
//

#ifndef APP_HTTPRPCCOMPONENT_H
#define APP_HTTPRPCCOMPONENT_H
#include"Client/Http.h"
#include"HttpListenComponent.h"

namespace Sentry
{
    class HttpHandlerClient;
    class HttpRpcComponent: public HttpListenComponent
    {
    public:
        HttpRpcComponent() = default;
    public:
        void OnRequest(std::shared_ptr<HttpHandlerClient> httpClient) final;
    private:

    };
}

#endif //APP_HTTPRPCCOMPONENT_H
