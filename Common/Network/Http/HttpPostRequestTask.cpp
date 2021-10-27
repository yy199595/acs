//
// Created by zmhy0073 on 2021/10/27.
//

#include "HttpPostRequestTask.h"
#include <Define/CommonDef.h>
namespace Sentry
{
    void HttpPostRequestTask::GetSendData(asio::streambuf &streambuf)
    {
        std::ostream os(&streambuf);
        os << "POST " << this->mPath << " HTTP/1.0\r\n";
        os << "Host: " << this->mHost << ":" << this->mPort << "\r\n";
        os << "Accept: */*\r\n";
        os << "Content-Length: " << this->mPostData->length() << "\r\n";
        os << "Content-Type: application/x-www-form-urlencoded\r\n";
        os << "Connection: close\r\n\r\n";
        os << (*this->mPostData);
    }

    void HttpPostRequestTask::OnReceiveBody(asio::streambuf &streambuf)
    {
        std::istream is(&streambuf);
        std::istreambuf_iterator<char> eos;
        this->mResponseData->append(std::istreambuf_iterator<char>(is), eos);
    }

    XCode HttpPostRequestTask::Post(const std::string &data, std::string &response)
    {
        this->mPostData = &data;
        this->mResponseData = &response;

        if(!this->AwaitInvoke())
        {
            return XCode::NoCoroutineContext;
        }
        if(this->mCode != XCode::Successful)
        {
            return this->mCode;
        }
        if(this->GetHttpCode() != HttpStatus::OK)
        {
            SayNoDebugError(this->GetError());
            return XCode::HttpResponseError;
        }
        return XCode::Successful;
    }
}