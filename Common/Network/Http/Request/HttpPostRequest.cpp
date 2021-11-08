//
// Created by zmhy0073 on 2021/11/1.
//
#include "HttpPostRequest.h"
#include <Network/NetworkHelper.h>
#include <Network/Http/HttpLocalsession.h>
#include<Network/Http/Content/HttpReadContent.h>
#include<Network/Http/Content/HttpWriteContent.h>
namespace GameKeeper
{

    HttpPostRequest::HttpPostRequest(HttpClientComponent *component)
        : HttpRequest(component)
    {

    }

    void HttpPostRequest::Clear()
    {
        this->mReadContent = nullptr;
        this->mWriteContent = nullptr;
    }

    void HttpPostRequest::OnReceiveBody(asio::streambuf &buf)
    {
        std::istream is(&buf);
        while(buf.size() > 0)
        {
           size_t size = is.readsome(this->mHandlerBuffer, 1024);
           this->mReadContent->OnReadContent(this->mHandlerBuffer, size);
        }
    }

    bool HttpPostRequest::Init(const std::string &url, HttpWriteContent &request, HttpReadContent &response)
    {
        if(!this->ParseUrl(url))
        {
            return false;
        }
        this->mReadContent = &response;
        this->mWriteContent = &request;
        return true;
    }

    bool HttpPostRequest::WriterToBuffer(std::ostream &os)
    {
        os << "POST " << this->GetPath() << " HTTP/1.0\r\n";
        os << "Host: " << this->GetHost() << ":" << this->GetPort() << "\r\n";
        os << "Accept: */*\r\n";
        os << "Content-Length: " << this->mWriteContent->GetContentSize() << "\r\n";
        os << "Content-Type: application/x-www-form-urlencoded\r\n";
        os << "Connection: close\r\n\r\n";
		return this->mWriteContent->GetContent(os);
    }
}