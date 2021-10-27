//
// Created by zmhy0073 on 2021/10/26.
//

#ifndef SENTRY_HTTPREMOTESESSION_H
#define SENTRY_HTTPREMOTESESSION_H
#include "HttpSessionBase.h"
namespace Sentry
{
    class HttpHandler;
    class HttpRemoteSession : public HttpSessionBase
    {
    public:
        HttpRemoteSession(ISocketHandler * socketHandler);

    public:
        SocketType GetSocketType() override { return SocketType::RemoteSocket;}
    protected:
        bool OnReceive(asio::streambuf &stream, const asio::error_code &err) override;
    private:
        HttpHandler * CreateHttpHandler(const std::string & method);
    private:
        int mReadCount;
        bool mIsReadBody;
        std::string mPath;
        std::string mMethod;
        std::string mVersion;
        HttpHandler * mHttpHandler;
    };
}

#endif //SENTRY_HTTPREMOTESESSION_H
