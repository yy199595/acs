//
// Created by zmhy0073 on 2021/10/25.
//

#ifndef SENTRY_HTTPGETREQUEST_H
#define SENTRY_HTTPGETREQUEST_H
#include "HttpRequest.h"

namespace Sentry
{
    class HttpGetRequest : public HttpRequest
    {
    public:
        HttpGetRequest(ISocketHandler * handler);
    public:
        XCode Get(const std::string & url, std::string & response);
    protected:
        void OnReceiveDone(bool hasError) override;
        void OnReceiveBody(asio::streambuf &strem) override;
        void GetRquestParame(asio::streambuf & strem) override;
    private:
        XCode mCode;
        std::string mPath;
        std::string mName;
        std::string mData;
        unsigned int mCoroutineId;
        CoroutineComponent * mCorComponent;
    };
}

#endif //SENTRY_HTTPGETREQUEST_H
