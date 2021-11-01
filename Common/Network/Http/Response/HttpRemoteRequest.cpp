//
// Created by zmhy0073 on 2021/11/1.
//

#include "HttpRemoteRequest.h"
#include <Network/Http/HttpClientComponent.h>
#include <Network/Http/HttpRemoteSession.h>
namespace Sentry
{

    HttpRemoteRequest::HttpRemoteRequest(HttpClientComponent *component, HttpRemoteSession *session)
        : mHttpSession(session), mHttpComponent(component)
    {

    }

    bool HttpRemoteRequest::OnReceiveHeard(asio::streambuf &buf, size_t size)
    {
        std::istream is(&buf);
        is >> this->mPath >> this->mVersion;
        this->ParseHeard(buf, size - (size - buf.size()));
        return true;
    }

    bool HttpRemoteRequest::OnSessionError(const asio::error_code &code)
    {
        return false;
    }
}