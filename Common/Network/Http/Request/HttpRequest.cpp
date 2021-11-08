//
// Created by zmhy0073 on 2021/11/1.
//

#include "HttpRequest.h"
#include <Define/CommonDef.h>
#include <Network/NetworkHelper.h>
namespace GameKeeper
{
    HttpRequest::HttpRequest(HttpClientComponent *component)
            : mHttpComponent(component)
    {

    }

    bool HttpRequest::ParseUrl(const std::string &url)
    {
        this->mHost.clear();
        this->mPort.clear();
        this->mPath.clear();
        return NetworkHelper::ParseHttpUrl(url, this->mHost, this->mPort, this->mPath);
    }

    void HttpRequest::Clear()
    {
        this->mHttpCode = 0;
        this->mPath.clear();
        this->mPort.clear();
        this->mHost.clear();
        this->mVersion.clear();
    }

    bool HttpRequest::OnReceiveHeard(asio::streambuf &streamBuf)
    {
        std::istream is(&streamBuf);
        is >> this->mVersion >> this->mHttpCode >> this->mError;

        this->ParseHeard(streamBuf);
#ifdef __DEBUG__
        GKDebugWarning(this->PrintHeard());
        GKDebugWarning(this->mVersion << " " << this->mHttpCode << " " << this->mError);
#endif
        return true;
    }
}