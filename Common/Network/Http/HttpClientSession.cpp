//
// Created by zmhy0073 on 2021/10/15.
//


#include <asio.hpp>

#include "HttpClientSession.h"
#include <Define/CommonDef.h>
#include <Network/Http/HttpRequestTask.h>
namespace Sentry
{
    HttpClientSession::HttpClientSession(ISocketHandler * handler)
        : SessionBase(handler), mResponseStream(&mResponseBuf)
    {

    }

    HttpClientSession::~HttpClientSession()
    {

    }

    void HttpClientSession::OnConnect(const asio::error_code &err)
    {

    }
    
    void HttpClientSession::ConnectHandler(const asio::error_code &err)
    {

    }

    void HttpClientSession::WriteHandler(const asio::error_code &err, const size_t size)
    {

    }

    void HttpClientSession::ReadHeadHandler(const asio::error_code &err, const size_t size)
    {
//        if (err)
//        {
//            SayNoDebugError(err.message());
//            this->mResponseHandler->Run(EHttpError::HttpReadError, mResponseStream);
//            return;
//        }
//        std::istreambuf_iterator<char> eos;
//        SayNoDebugError(string(std::istreambuf_iterator<char>(this->mResponseStream), eos));
//        asio::async_read(*this->mHttpSocket, this->mResponseBuf, asio::transfer_at_least(1),
//                         std::bind(&HttpClientSession::ReadBodyHandler, this, args1, args2));
    }


    void HttpClientSession::ReadBodyHandler(const asio::error_code &err, const size_t size)
    {

    }
}