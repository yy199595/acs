//
// Created by zmhy0073 on 2021/11/1.
//
#include "HttpPostRequest.h"
#include <NetworkHelper.h>
#include <Http/HttpLocalsession.h>
#include<Http/Content/HttpReadContent.h>
#include<Network/Http/Content/HttpWriteContent.h>
namespace GameKeeper
{

    HttpPostRequest::HttpPostRequest(HttpComponent *component)
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

    bool HttpPostRequest::Init(const std::string &url, HttpWriteContent *request, HttpReadContent *response)
    {
        if(!this->ParseUrl(url))
        {
            return false;
        }
        this->mReadContent = response;
        this->mWriteContent = request;
        return true;
    }

    void HttpPostRequest::WriteHead(std::ostream &os)
    {
        os << "POST " << this->GetPath() << " " << HttpVersion << "\r\n";
        os << "Host: " << this->GetHost() << ":" << this->GetPort() << "\r\n";
        os << "Accept: */*\r\n";
        this->mWriteContent->WriteHead(os);
        os << "Connection: close\r\n\r\n";
    }

    bool HttpPostRequest::WriteBody(std::ostream &os)
    {
        if(this->mWriteContent == nullptr)
        {
            return true;
        }
        return this->mWriteContent->WriteBody(os);
    }
}