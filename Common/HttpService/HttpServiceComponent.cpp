//
// Created by zmhy0073 on 2021/11/2.
//

#include "HttpServiceComponent.h"

namespace GameKeeper
{
    bool HttpServiceComponent::Awake()
    {
        return true;
    }

    HttpServiceMethod *HttpServiceComponent::GetMethod(const std::string &path)
    {
        auto iter = this->mMethodMap.find(path);
        if(iter == this->mMethodMap.end())
        {
            return nullptr;
        }
        return iter->second;
    }
}