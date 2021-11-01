//
// Created by zmhy0073 on 2021/11/1.
//
#include "HttpLocalPostRequest.h"
#include <Network/NetworkHelper.h>
#include <Network/Http/HttpLocalsession.h>
namespace GameKeeper
{

    HttpLocalPostRequest::HttpLocalPostRequest(HttpClientComponent *component)
        : HttpLocalRequest(component)
    {
        this->mCorId = 0;
        this->mResponse = nullptr;
        this->mPostData = nullptr;
    }
    XCode HttpLocalPostRequest::Post(const std::string &url, const std::string &data, std::string &response)
    {
        this->mPostData = &data;
        this->mResponse = &response;
        return this->StartHttpRequest(url);
    }

    bool HttpLocalPostRequest::OnReceiveBody(asio::streambuf &buf)
    {
        std::istream is(&buf);
        while(buf.size() > 0)
        {
           size_t size = is.readsome(this->mHandlerBuffer, 1024);
           this->mResponse->append(this->mHandlerBuffer, size);
        }
        return true;
    }

    bool HttpLocalPostRequest::WriterToBuffer(std::ostream &os)
    {
        os << "POST " << this->mPath << " HTTP/1.0\r\n";
        os << "Host: " << this->mHost << ":" << this->mPort << "\r\n";
        os << "Accept: */*\r\n";
        os << "Content-Length: " << this->mPostData->length() << "\r\n";
        os << "Content-Type: application/x-www-form-urlencoded\r\n";
        os << "Connection: close\r\n\r\n";
        os << (*this->mPostData);
		return true;
    }

}