//
// Created by zmhy0073 on 2021/11/1.
//

#include"HttpRequest.h"
#include<Define/CommonLogDef.h>
#include<NetworkHelper.h>
#include"Core/App.h"
namespace GameKeeper
{
    HttpRequest::HttpRequest(const std::string &url)
    {
        this->mHasParseError = !NetworkHelper::ParseHttpUrl(
                url, this->mHost, this->mPort, this->mPath);
    }
}