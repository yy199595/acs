//
// Created by zmhy0073 on 2021/10/15.
//


#include <asio.hpp>

#include "HttpClientSession.h"
#include <Define/CommonDef.h>
#include <Network/Http/HttpRequestTask.h>
namespace Sentry
{
    HttpClientSession::HttpClientSession(ISocketHandler * handler, HttpRequestTask & task)
        : SessionBase(handler), mHttpTask(task), mResponseStream(&mResponseBuf)
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