//
// Created by zmhy0073 on 2021/11/1.
//

#include "HttpLocalRequest.h"
#include <Define/CommonDef.h>
#include <Core/App.h>
#include <Network/Http/HttpLocalsession.h>
#include <Network/NetworkHelper.h>
namespace GameKeeper
{
    HttpLocalRequest::HttpLocalRequest(HttpClientComponent *component)
            : mHttpComponent(component)
    {

    }

    XCode HttpLocalRequest::StartHttpRequest(const std::string & url)
    {
        if (!NetworkHelper::ParseHttpUrl(url, this->mHost, this->mPort, this->mPath))
        {
            return XCode::HttpUrlParseError;
        }
        HttpLocalSession httpSession(mHttpComponent, this);
        httpSession.StartConnectHost(this->mHost, this->mPort);
        App::Get().GetCorComponent()->YieldReturn(this->mCorId);

        if(this->mCode != XCode::Successful)
        {
            return this->mCode;
        }
        if(this->mHttpCode != 200)
        {
            return XCode::HttpResponseError;
        }
        return XCode::Successful;
    }

    void HttpLocalRequest::SetCode(XCode code)
    {
        this->mCode = code;
        CoroutineComponent *corComponent = App::Get().GetCorComponent();
        MainTaskScheduler &taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.AddMainTask(&CoroutineComponent::Resume, corComponent, this->mCorId);
    }

    void HttpLocalRequest::OnSessionError(const asio::error_code &code)
    {
        this->SetCode(code == asio::error::eof
                      ? XCode::Successful : XCode::HttpNetWorkError);
    }

    bool HttpLocalRequest::OnReceiveHeard(asio::streambuf &buf, size_t size)
    {
        std::istream is(&buf);
        size_t size1 = buf.size();
        is >> this->mVersion >> this->mHttpCode >> this->mError;
        size_t size2 = size - (size1 - buf.size());
#ifdef __DEBUG__
        GKDebugWarning(this->mVersion << " " << this->mHttpCode << " " << this->mError);
#endif
        this->ParseHeard(buf, size2);
        return true;
    }
}