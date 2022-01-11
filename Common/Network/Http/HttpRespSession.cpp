//
// Created by zmhy0073 on 2021/10/26.
//

#include "HttpRespSession.h"
#include <Core/App.h>
#include "Http/Component/HttpClientComponent.h"
#include <Network/Http/Response/HttpGettHandler.h>
#include <Network/Http/Response/HttpPostHandler.h>
#include <Method/HttpServiceMethod.h>
namespace GameKeeper
{
    HttpRespSession::HttpRespSession(HttpClientComponent *component)
    {
        this->mWriterCount = 0;
        this->mHttpHandler = nullptr;
        this->mSocketProxy = nullptr;
        this->mHttpComponent = component;
        this->mHandlerMap["GET"] = new HttpGettHandler(component);
        this->mHandlerMap["POST"] = new HttpPostHandler(component);
    }

    HttpRespSession::~HttpRespSession() noexcept
    {
        auto iter = this->mHandlerMap.begin();
        for(; iter != this->mHandlerMap.end(); iter++)
        {
            delete iter->second;
        }
        this->mHandlerMap.clear();
    }

    void HttpRespSession::WriterToBuffer(std::ostream & os)
    {
        if(this->mHttpHandler == nullptr)
        {
            HttpStatus code = HttpStatus::BAD_REQUEST;
            os << HttpVersion << (int) code << " " << HttpStatusToString(code) << "\r\n";
            os << "Server: " << "GameKeeper" << "\r\n";
            os << "Connection: " << "close" << "\r\n\r\n";
        }
    }

    void HttpRespSession::Start(SocketProxy *socketProxy)
    {
        delete this->mSocketProxy;
        this->mSocketProxy = socketProxy;
        this->StartReceiveHead();
    }

	bool HttpRespSession::OnReceiveHead(asio::streambuf & streamBuf)
    {
        std::istream is(&streamBuf);
        is >> this->mMethod;
        this->mHttpHandler = nullptr;
        auto iter = this->mHandlerMap.find(this->mMethod);
        if (iter == this->mHandlerMap.end())
        {
            LOG_ERROR("not find http method {0}", this->mMethod);
            return false;
        }
        this->mHttpHandler = iter->second;
        return this->mHttpHandler->OnReceiveHead(streamBuf);
    }

    bool HttpRespSession::OnReceiveBody(asio::streambuf &buf)
    {
        std::istream is(&buf);
        char buffer[256] = { 0 };
        while(buf.size() > 0)
        {
            size_t size = is.readsome(buffer, 256);
        }
        return true;
    }

    void HttpRespSession::OnComplete(XCode code)
    {

    }

    void HttpRespSession::OnWriterAfter(XCode code)
    {
#ifdef __DEBUG__
        if (this->mHttpHandler != nullptr)
        {
            long long endTime = Helper::Time::GetMilTimestamp();
            LOG_DEBUG("http call {0}.{1} user time = {2}s",
                      this->mHttpHandler->GetComponent(), this->mHttpHandler->GetMethod(),
                      ((endTime - this->mHttpHandler->GetStartTime()) / 1000.0f));
        }
#endif
        if(this->mHttpHandler != nullptr)
        {
            this->mHttpHandler->Clear();
        }
        delete this;
    }
}