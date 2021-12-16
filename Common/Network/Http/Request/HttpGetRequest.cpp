#include"HttpGetRequest.h"
#include<Define/CommonLogDef.h>
#include<NetworkHelper.h>
#include"Core/App.h"
#include<Http/Content/HttpReadContent.h>
namespace GameKeeper
{
    HttpGetRequest::HttpGetRequest(const std::string &url)
        : HttpRequest(url)
    {

    }

    void HttpGetRequest::WriteToSendBuffer(std::ostream &os)
    {
        os << "GET " << this->GetPath() << " " << HttpVersion << "\r\n";
        os << "Host: " << this->GetHost() << "\r\n";
        os << "Accept: */*\r\n";
        os << "Connection: close\r\n\r\n";
    }
}