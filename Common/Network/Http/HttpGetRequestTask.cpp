//
// Created by zmhy0073 on 2021/10/26.
//

#include "HttpGetRequestTask.h"
#include <ostream>
#include <Network/NetworkHelper.h>
namespace Sentry
{
    HttpGetRequestTask::HttpGetRequestTask(const std::string &url)
        : HttpRequestTask(url)
    {

    }

    XCode HttpGetRequestTask::Get(std::string &response)
    {
        this->mResponse = &response;
        if(!this->AwaitInvoke())
        {
            return XCode::NoCoroutineContext;
        }
        if(this->mCode != XCode::Successful)
        {
            return this->mCode;
        }
        return this->mHttpCode == 200 ? XCode::Successful : XCode::HttpResponseError;
    }

    void HttpGetRequestTask::GetSendData(asio::streambuf &streambuf)
    {
        std::ostream os(&streambuf);
        os << "GET " << mPath << " HTTP/1.0\r\n";
        os << "Host: " << mHost << "\r\n";
        os << "Accept: */*\r\n";
        os << "Connection: close\r\n\r\n";
    }

    void HttpGetRequestTask::OnReceiveBody(asio::streambuf &streambuf)
    {
        std::istream is(&streambuf);
        std::istreambuf_iterator<char> eos;
        this->mResponse->append(std::istreambuf_iterator<char>(is), eos);
    }
}