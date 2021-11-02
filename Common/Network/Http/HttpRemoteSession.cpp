//
// Created by zmhy0073 on 2021/10/26.
//

#include "HttpRemoteSession.h"
#include <Network/Http/HttpClientComponent.h>
#include <Network/Http/Response/HttpRemoteRequest.h>
namespace GameKeeper
{
    HttpRemoteSession::HttpRemoteSession(HttpClientComponent *component)
        : HttpSessionBase(component)
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

    void HttpRemoteSession::OnSendHttpMessageAfter()
    {
        this->DestoryHandler();
    }

    void HttpRemoteSession::DestoryHandler()
    {
        delete this->mHttpHandler;
        delete this;
    }

    bool HttpRemoteSession::OnReceiveBody(asio::streambuf &buf, const asio::error_code &code)
    {
        if(code)
        {
            this->DestoryHandler();
            this->OnSocketError(code);
            return false;
        }
        if (this->mHttpHandler == nullptr)
        {
            return false;
        }
        return this->mHttpHandler->OnReceiveBody(buf);
    }

    void HttpRemoteSession::OnSessionEnable()
    {
        this->StartReceive();
    }

    bool HttpRemoteSession::OnReceiveHeard(asio::streambuf &buf, size_t size, const asio::error_code &code)
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