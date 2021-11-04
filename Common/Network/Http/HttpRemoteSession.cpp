//
// Created by zmhy0073 on 2021/10/26.
//

#include "HttpRemoteSession.h"
#include <Core/App.h>
#include <Scene/ProtocolComponent.h>
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
        GKAssertRetFalse_F(this->mHttpHandler);
        return this->mHttpHandler->WriterToBuffer(os);
    }

    void HttpRemoteSession::OnWriteAfter(XCode code)
    {
        GKAssertRet_F(this->mHttpHandler);
        this->mHttpHandler->OnWriterAfter(code);
    }

    void HttpRemoteSession::OnReceiveBody(asio::streambuf &buf)
    {
        GKAssertRet_F(this->mHttpHandler);
        this->mHttpHandler->OnReceiveBody(buf);
    }

    void HttpRemoteSession::OnReceiveBodyAfter(XCode code)
    {
        GKAssertRet_F(this->mHttpHandler);
        this->mHttpHandler->OnReceiveBodyAfter(code);
    }

    void HttpRemoteSession::OnReceiveHeardAfter(XCode code)
    {
        GKAssertRet_F(this->mHttpHandler);
        this->mHttpHandler->OnReceiveHeardAfter(code);
    }

    void HttpRemoteSession::OnSessionEnable()
    {
        this->StartReceiveHeard();
    }

    bool HttpRemoteSession::OnReceiveHeard(asio::streambuf &buf, size_t size)
    {
        std::string method;
        std::istream is(&buf);
        is >> method >> this->mPath >> this->mVersion;
        this->mHttpHandler = this->mHttpComponent->CreateMethodHandler(method, this);
        if(this->mHttpHandler == nullptr)
        {
            GKDebugFatal("create http method " << method << " handler failure");
            return false;
        }
        size_t size1 = size - (size - buf.size());
        return this->mHttpHandler->OnReceiveHeard(buf, size1);;
    }
}