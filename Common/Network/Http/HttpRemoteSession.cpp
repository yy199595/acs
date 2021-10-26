//
// Created by zmhy0073 on 2021/10/26.
//

#include "HttpRemoteSession.h"
#include <Network/Http/HttpGetHandler.h>
namespace Sentry
{
    HttpRemoteSession::HttpRemoteSession(ISocketHandler *socketHandler)
        : HttpSessionBase(socketHandler)
    {
        this->mReadCount = 0;
        this->mIsReadBody = false;
        this->mHttpHandler = nullptr;
    }

    bool HttpRemoteSession::OnReceive(asio::streambuf &stream, const asio::error_code &err)
    {
        if (this->mReadCount == 0)
        {
            std::istream is(&stream);
            is >> this->mMethod >>this->mPath >> this->mVersion;
            this->mHttpHandler = this->CreateHttpHandler(this->mMethod);
            if(mHttpHandler == nullptr)
            {
                return false;
            }
        }
        this->mReadCount++;
        if(!this->mIsReadBody)
        {
            std::istream is(&stream);
            const char *data = asio::buffer_cast<const char *>(stream.data());
            const char *pos = strstr(data, "\r\n\r\n");
            if(pos == nullptr)
            {
                return true;
            }
            size_t size = pos - data + strlen("\r\n\r\n");
            if (size != 0)
            {
                is.ignore(size);
                this->mIsReadBody = true;
                this->mHttpHandler->OnReceiveHeard(this->mPath, this->mVersion, std::string(data, size));
            }
        }
        if(this->mIsReadBody)
        {
            return this->mHttpHandler->OnReceive(stream, err);
        }
        return true;
    }

    HttpHandler *HttpRemoteSession::CreateHttpHandler(const std::string &method)
    {
        if(method.compare("GET"))
        {
            return new HttpGetHandler(this);
        }
        return nullptr;
    }
}