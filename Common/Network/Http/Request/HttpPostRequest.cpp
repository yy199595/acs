//
// Created by zmhy0073 on 2021/11/1.
//
#include "HttpPostRequest.h"
#include <Network/NetworkHelper.h>
#include <Network/Http/HttpLocalsession.h>
namespace GameKeeper
{

    HttpPostRequest::HttpPostRequest(HttpClientComponent *component)
        : HttpRequest(component)
    {
        this->mCorId = 0;
        this->mResponse = nullptr;
        this->mPostContent = nullptr;
    }
    XCode HttpPostRequest::Post(const std::string &url, HttpWriteContent & content, std::string &response)
    {
        this->mResponse = &response;
        this->mPostContent = &content;
        return this->StartHttpRequest(url);
    }

    void HttpPostRequest::OnReceiveBody(asio::streambuf &buf)
    {
        std::istream is(&buf);
        while(buf.size() > 0 && this->mPostContent)
        {
           size_t size = is.readsome(this->mHandlerBuffer, 1024);
           this->mResponse->append(this->mHandlerBuffer, size);
        }
    }

    bool HttpPostRequest::WriterToBuffer(std::ostream &os)
    {
        os << "POST " << this->mPath << " HTTP/1.0\r\n";
        os << "Host: " << this->mHost << ":" << this->mPort << "\r\n";
        os << "Accept: */*\r\n";
        os << "Content-Length: " << this->mPostContent->GetContentSize() << "\r\n";
        os << "Content-Type: application/x-www-form-urlencoded\r\n";
        os << "Connection: close\r\n\r\n";
		return this->mPostContent->GetContent(os);
    }
}