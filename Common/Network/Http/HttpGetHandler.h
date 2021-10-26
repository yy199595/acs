//
// Created by zmhy0073 on 2021/10/26.
//

#ifndef SENTRY_HTTPGETHANDLER_H
#define SENTRY_HTTPGETHANDLER_H
#include "HttpHandler.h"
namespace Sentry
{
    class HttpGetHandler : public HttpHandler
    {
    public:
        using HttpHandler::HttpHandler;
    protected:
        void OnReceiveDone() override;
        void OnReceiveBody(asio::streambuf &streambuf) override;
    };
}

#endif //SENTRY_HTTPGETHANDLER_H

