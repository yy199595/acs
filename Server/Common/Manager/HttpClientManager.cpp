//
// Created by 64658 on 2021/8/5.
//

#include "HttpClientManager.h"
#include <Coroutine/CoroutineManager.h>
namespace Sentry
{
    bool HttpClientManager::OnInit()
    {
        this->mCorManager = this->GetManager<CoroutineManager>();
        return true;
    }

    void HttpClientManager::OnSystemUpdate()
    {

    }

    XCode HttpClientManager::Get(const std::string & url, std::string & json, int timeout)
    {
        if(this->mCorManager->IsInMainCoroutine())
        {
            return XCode::NoCoroutineContext;
        }

        return XCode::Successful;
    }

}