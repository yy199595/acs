//
// Created by zmhy0073 on 2021/11/1.
//

#ifndef SENTRY_HTTPREMOTEGETREQUEST_H
#define SENTRY_HTTPREMOTEGETREQUEST_H
#include "HttpRemoteRequest.h"

namespace Sentry
{
    class HttpRemoteGetRequest : public HttpRemoteRequest
    {
    public:
        HttpRemoteGetRequest() = default;
        ~HttpRemoteGetRequest() override = default;
    protected:
         bool WriterToBuffer(std::ostream & os) override;
         bool OnReceiveBody(asio::streambuf & buf) override;

    private:
        std::string mPage;
        std::string mParamater;
    };
}
#endif //SENTRY_HTTPREMOTEGETREQUEST_H
