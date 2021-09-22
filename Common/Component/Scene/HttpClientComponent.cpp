//
// Created by 64658 on 2021/8/5.
//

#include "HttpClientComponent.h"
#include <Coroutine/CoroutineComponent.h>

namespace Sentry
{
    bool HttpClientComponent::Awake()
    {
      
      return true;
    }

    void HttpClientComponent::OnSystemUpdate()
    {

    }

    XCode HttpClientComponent::Get(const std::string &url, std::string &json, int timeout)
    {
      if (this->mCorComponent->IsInMainCoroutine())
      {
        return XCode::NoCoroutineContext;
      }

      return XCode::Successful;
    }

}