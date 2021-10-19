//
// Created by zmhy0073 on 2021/10/15.
//

#include "HttpClientSession.h"
#include <asio.hpp>
#include <Define/CommonDef.h>
namespace Sentry
{
    HttpClientSession::HttpClientSession(AsioContext &io,const std::string & host, const std::string & port)
        : mHttpContext(io), mResponseStream(&mResponseBuf), mResolver(io), mQuery(host, port)
    {
        this->mResponseHandler = nullptr;
    }

    HttpClientSession::~HttpClientSession()
    {
        if(this->mResponseHandler)
        {
            delete this->mResponseHandler;
        }
        if(this->mHttpSocket != nullptr && this->mHttpSocket->is_open())
        {
            asio::error_code err;
            this->mHttpSocket->close(err);
			SayNoDebugError(err.message());
        }
    }

    void HttpClientSession::Request(SharedMessage message, IHttpReponseHandler *handler)
    {
        if (message == nullptr || message->empty() || handler == nullptr)
        {
            return;
        }
        this->mRequestMessage = message;
        this->mResponseHandler = handler;
        this->mResolver.async_resolve(this->mQuery, std::bind(&HttpClientSession::ResolverHandler, this, args1, args2));
    }

    void HttpClientSession::ResolverHandler(const asio::error_code &err, tcp::resolver::iterator iterator)
    {
        if(err)
        {
            SayNoDebugError(err.message());
            this->mResponseHandler->Run(EHttpError::HttpResolverError, this->mResponseStream);
            return;
        }
        this->mHttpSocket = std::make_shared<asio::ip::tcp::socket>(this->mHttpContext);
        //this->mHttpSocket->async_connect(*iterator, std::bind(&HttpClientSession::ConnectHandler, this, args1));
        asio::async_connect(*this->mHttpSocket, iterator, std::bind(&HttpClientSession::ConnectHandler, this, args1));
    }

    void HttpClientSession::ConnectHandler(const asio::error_code &err)
    {
        if (err)
        {
            SayNoDebugError(err.message());
            this->mResponseHandler->Run(EHttpError::HttpConnectError, mResponseStream);
            return;
        }
        SayNoDebugLog("connect http host " << this->mHttpSocket->remote_endpoint().address()
                                           << ":" << this->mHttpSocket->remote_endpoint().port() << " successful")
        this->mHttpSocket->async_write_some(
                asio::buffer(this->mRequestMessage->c_str(), this->mRequestMessage->size()),
                std::bind(&HttpClientSession::WriteHandler, this, args1, args2));
    }

    void HttpClientSession::WriteHandler(const asio::error_code &err, const size_t size)
    {
        if(err)
        {
            SayNoDebugError(err.message());
            this->mResponseHandler->Run(EHttpError::HttpWriteError, mResponseStream);
            return;
        }
        this->mRequestMessage->clear();
		/*/asio::async_read_until(*this->mHttpSocket, this->mResponseBuf, "\r\n\r\n", 
			std::bind(&HttpClientSession::ReadHeadHandler, this, args1, args2));*/
        asio::async_read(*this->mHttpSocket, this->mResponseBuf, asio::transfer_at_least(1),
                         std::bind(&HttpClientSession::ReadBodyHandler, this, args1, args2));
    }

    void HttpClientSession::ReadHeadHandler(const asio::error_code &err, const size_t size)
    {
        if (err)
        {
            SayNoDebugError(err.message());
            this->mResponseHandler->Run(EHttpError::HttpReadError, mResponseStream);
            return;
        }
        std::istreambuf_iterator<char> eos;
        SayNoDebugError(string(std::istreambuf_iterator<char>(this->mResponseStream), eos));
        asio::async_read(*this->mHttpSocket, this->mResponseBuf, asio::transfer_at_least(1),
                         std::bind(&HttpClientSession::ReadBodyHandler, this, args1, args2));
    }


    void HttpClientSession::ReadBodyHandler(const asio::error_code &err, const size_t size)
    {
        if (err == asio::error::eof)
        {
            std::istreambuf_iterator<char> eos;
            SayNoDebugWarning("size = " << this->mResponseBuf.size() << "\n"
                                        << string(std::istreambuf_iterator<char>(this->mResponseStream), eos));
            this->mResponseHandler->Run(EHttpError::HttpSuccessful, mResponseStream);
        }	
		else if(err)
        {
            SayNoDebugError(err.message());
            this->mResponseHandler->Run(EHttpError::HttpResponseError, mResponseStream);
        }
        else
        {
            asio::async_read(*this->mHttpSocket, this->mResponseBuf, asio::transfer_at_least(1),
                             std::bind(&HttpClientSession::ReadBodyHandler, this, args1, args2));
        }
    }
}