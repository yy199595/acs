//
// Created by zmhy0073 on 2021/11/1.
//

#ifndef SENTRY_HTTPLOCALPOSTREQUEST_H
#define SENTRY_HTTPLOCALPOSTREQUEST_H
#include "HttpLocalRequest.h"
namespace Sentry
{
    class HttpLocalPostRequest : public HttpLocalRequest
    {
    public:
        HttpLocalPostRequest(HttpClientComponent * component);
        ~HttpLocalPostRequest() override = default;
    public:
        XCode Post(const std::string & url, const std::string & data, std::string & response);
    protected:
        bool WriterToBuffer(std::ostream & os) override;
        bool OnReceiveBody(asio::streambuf & buf) override;
    private:
        unsigned int mCorId;
        std::string * mResponse;
        const std::string * mPostData;
    };
}
#endif //SENTRY_HTTPLOCALPOSTREQUEST_H
