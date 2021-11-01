//
// Created by zmhy0073 on 2021/10/26.
//

#include "HttpRemoteSession.h"
#include <Network/Http/HttpClientComponent.h>
namespace Sentry
{
    HttpRemoteSession::HttpRemoteSession(HttpClientComponent *component)
        : HttpSessionBase(component)
    {
        this->mReadCount = 0;
        this->mIsReadBody = false;
        this->mHttpHandler = nullptr;
        this->mHttpComponent = component;
    }

    bool HttpRemoteSession::WriterToBuffer(std::ostream &os)
    {

        return true;
    }

    bool HttpRemoteSession::OnReceiveBody(asio::streambuf &buf, const asio::error_code &code)
    {
        return true;
    }

    bool HttpRemoteSession::OnReceiveHeard(asio::streambuf &buf, size_t size, const asio::error_code &code)
    {
        return true;
    }
}