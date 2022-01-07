//
// Created by zmhy0073 on 2021/11/1.
//
#include "HttpPostRequest.h"
#include <NetworkHelper.h>
#include <Http/HttpReqSession.h>
#include<Http/Content/HttpReadContent.h>
#include<Network/Http/Content/HttpWriteContent.h>
namespace GameKeeper
{
    HttpPostRequest::HttpPostRequest(const std::string &url, const std::string &content)
        : HttpRequest(url)
    {
        this->mWriteContent = std::move(content);
    }

    void HttpPostRequest::WriteToSendBuffer(std::ostream &os)
    {
        os << "POST " << this->GetPath() << " " << HttpVersion << "\r\n";
        os << "Host: " << this->GetHost() << ":" << this->GetPort() << "\r\n";
        os << "Content-Type: text/plain; charset=utf-8" << "\r\n";
        os << "Content-Length: " << this->mWriteContent.size()<< "\r\n";
        os << "Accept: */*\r\n";
        os << "Connection: close\r\n\r\n";
        os.write(this->mWriteContent.c_str(), this->mWriteContent.size());
    }
}