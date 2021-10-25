//
// Created by zmhy0073 on 2021/10/25.
//
#include <Core/App.h>
#include "HttpGetRequest.h"

namespace Sentry
{
    HttpGetRequest::HttpGetRequest(ISocketHandler * handler)
            : HttpRequest(handler, "HttpGetRequest")
    {
        this->mCorComponent = App::Get().GetCoroutineComponent();
    }

    void HttpGetRequest::GetRquestParame(asio::streambuf & strem)
    {
        std::ostream os(&strem);
        os << "GET " << mPath << " HTTP/1.0\r\n";
        os << "Host: " << mHost << "\r\n";
        os << "Accept: */*\r\n";
        os << "Connection: close\r\n\r\n";
    }

    void HttpGetRequest::OnReceiveDone(bool hasError)
    {
        this->mCode = hasError ? XCode::HttpNetWorkError : XCode::Successful;
        this->mMainTaskPool.AddMainTask(&CoroutineComponent::Resume, this->mCorComponent, this->mCoroutineId);
    }

    void HttpGetRequest::OnReceiveBody(asio::streambuf &strem)
    {
        std::istream is(&strem);
        std::istreambuf_iterator<char> eos;
        SayNoDebugWarning(strem.size());
        this->mData.append(std::istreambuf_iterator<char>(is), eos);
    }


    XCode HttpGetRequest::Get(const std::string & url, std::string & response)
    {
        if (!NetworkHelper::ParseHttpUrl(url, this->mHost, this->mPort, this->mPath))
        {
            return XCode::HttpUrlParseError;
        }
        if (!this->ConnectRemote())
        {
            return XCode::HttpNetWorkError;
        }
        this->mCorComponent->YieldReturn(this->mCoroutineId);
        if (this->mCode != XCode::Successful)
        {
            return this->mCode;
        }
        if(this->GetHttpCode() != 200)
        {
            return XCode::HttpResponseError;
        }
        response = this->mData;
        return XCode::Successful;
    }

}