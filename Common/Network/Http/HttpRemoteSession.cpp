//
// Created by zmhy0073 on 2021/10/26.
//

#include "HttpRemoteSession.h"
#include <Network/Http/HttpClientComponent.h>
#include <Network/Http/Response/HttpRemoteRequestHandler.h>
namespace GameKeeper
{
    HttpRemoteSession::HttpRemoteSession(HttpClientComponent *component)
        : HttpSessionBase(component, "RemoteHttpSession")
    {
        this->mHttpHandler = nullptr;
        this->mHttpComponent = component;
    }

    bool HttpRemoteSession::WriterToBuffer(std::ostream &os)
    {
        if(this->mHttpHandler == nullptr)
        {
            return true;
        }
        return this->mHttpHandler->WriterToBuffer(os);
    }

    void HttpRemoteSession::OnWriteAfter()
    {
        if(this->mHttpHandler == nullptr)
        {
            assert(false);
        }
        this->mHttpHandler->OnWriterAfter();
    }

    void HttpRemoteSession::OnReceiveBody(asio::streambuf &buf)
    {
        if (this->mHttpHandler == nullptr)
        {
            return;
        }
        this->mHttpHandler->OnReceiveBody(buf);
    }

    void HttpRemoteSession::OnSessionEnable()
    {
        this->StartReceiveHeard();
    }

    void HttpRemoteSession::OnSocketError(const asio::error_code &err)
    {
        if(this->mHttpHandler== nullptr)
        {
            return;
        }
        this->mHttpHandler->OnSessionError(err);
    }

    bool HttpRemoteSession::OnReceiveHeard(asio::streambuf &buf, size_t size)
    {
        std::istream is(&buf);
        is >> this->mMethod;
        this->mHttpHandler = this->mHttpComponent->CreateMethodHandler(this->mMethod, this);
        if (this->mHttpHandler == nullptr)
        {
            return false;
        }
        size_t size1 = size - (size - buf.size());
        return this->mHttpHandler->OnReceiveHeard(buf, size1);;
    }
}