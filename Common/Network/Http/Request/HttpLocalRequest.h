//
// Created by zmhy0073 on 2021/11/1.
//

#ifndef SENTRY_HTTPLOCALREQUEST_H
#define SENTRY_HTTPLOCALREQUEST_H
#include <Network/Http/HttpHandlerBase.h>
namespace Sentry
{
    class HttpClientComponent;
    class HttpLocalRequest : public HttpHandlerBase
    {
    public:
        HttpLocalRequest(HttpClientComponent * component);

        ~HttpLocalRequest() override = default;

    public:

        const std::string & GetError() { return this->mError;}

        const std::string &GetVersion() { return this->mVersion; }

        HttpStatus GetHttpCode() const { return (HttpStatus) mHttpCode; }

    protected:

        XCode StartHttpRequest(const std::string & url);

        void SetCode(XCode code);

        bool OnSessionError(const asio::error_code &code) override;

        bool OnReceiveHeard(asio::streambuf &buf, size_t size) override;


    protected:
        std::string mHost;
        std::string mPort;
        std::string mPath;
        unsigned int mCorId;
    protected:
        XCode mCode;
        int mHttpCode;
        std::string mError;
        std::string mVersion;
        HttpClientComponent * mHttpComponent;
    };
}
#endif //SENTRY_HTTPLOCALREQUEST_H
