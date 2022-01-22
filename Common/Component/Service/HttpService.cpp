//
// Created by yjz on 2022/1/23.
//
#include"HttpService.h"

namespace Sentry
{
    std::shared_ptr<RapidJsonWriter> HttpService::Invoke(
            const std::string &name, std::shared_ptr<RapidJsonReader> request)
    {
        std::shared_ptr<RapidJsonWriter> response(new RapidJsonWriter());
        auto iter = this->mHttpMethodMap.find(name);
        if(iter == this->mHttpMethodMap.end())
        {
            response->Add("code", (int)XCode::CallServiceNotFound);
            return response;
        }
        HttpServiceMethod * httpServiceMethod = iter->second;
        XCode code = httpServiceMethod->Invoke(request, response);
        response->Add("code", (int)code);
        return response;
    }
}