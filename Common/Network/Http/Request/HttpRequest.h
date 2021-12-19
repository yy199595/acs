//
// Created by zmhy0073 on 2021/11/1.
//

#ifndef GameKeeper_HTTPLOCALREQUEST_H
#define GameKeeper_HTTPLOCALREQUEST_H
#include <Http/HttpHandlerBase.h>
namespace GameKeeper
{
    class HttpReqSession;

    class HttpClientComponent;

    class HttpRequest
    {
    public:
        explicit HttpRequest(const std::string & url);
        virtual ~HttpRequest() = default;

    public:
        const std::string &GetPath() const { return this->mPath; }

        const std::string &GetHost() const { return this->mHost; }

        const std::string &GetPort() const { return this->mPort; }

        bool HasParseError() const { return this->mHasParseError;}

    public:
        virtual void WriteToSendBuffer(std::ostream & os) = 0;
    private:
        std::string mPath;
        std::string mHost;
        std::string mPort;
        bool mHasParseError;
    };
}
#endif //GameKeeper_HTTPLOCALREQUEST_H
