//
// Created by zmhy0073 on 2021/11/1.
//

#ifndef GameKeeper_HTTPLOCALREQUEST_H
#define GameKeeper_HTTPLOCALREQUEST_H
#include <Network/Http/HttpHandlerBase.h>
namespace GameKeeper
{
    class HttpLocalSession;

    class HttpClientComponent;

    class HttpRequest : public HttpHandlerBase
    {
    public:
        HttpRequest(HttpClientComponent *component);

        ~HttpRequest() override = default;

    public:

        virtual void Clear() override;

        const std::string &GetError() const { return this->mError; }

        const std::string &GetVersion() const { return this->mVersion; }

        const std::string &GetPath() const { return this->mPath; }

        const std::string &GetHost() const { return this->mHost; }

        const std::string &GetPort() const { return this->mPort; }

        HttpStatus GetHttpCode() const { return (HttpStatus) mHttpCode; }

        bool ParseUrl(const std::string &url);

        virtual void OnReceiveBody(asio::streambuf & buf) = 0;

    public:
        bool OnReceiveHeard(asio::streambuf &buf) override;

    protected:
        int mHttpCode;
        std::string mError;
        std::string mVersion;
        HttpClientComponent *mHttpComponent;
    private:
        std::string mPath;
        std::string mHost;
        std::string mPort;
    };
}
#endif //GameKeeper_HTTPLOCALREQUEST_H
