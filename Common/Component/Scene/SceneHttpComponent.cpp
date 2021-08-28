//
// Created by 64658 on 2021/8/5.
//

#include "SceneHttpComponent.h"
#include <Coroutine/CoroutineComponent.h>

namespace Sentry
{
    bool SceneHttpComponent::Awake()
    {
      
      return true;
    }

    void SceneHttpComponent::OnSystemUpdate()
    {

    }

    XCode SceneHttpComponent::Get(const std::string &url, std::string &json, int timeout)
    {
      if (this->mCorComponent->IsInMainCoroutine())
      {
        return XCode::NoCoroutineContext;
      }

      return XCode::Successful;
    }

}