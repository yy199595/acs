//
// Created by zmhy0073 on 2021/11/2.
//

#ifndef GAMEKEEPER_HTTPSERVICECOMPONENT_H
#define GAMEKEEPER_HTTPSERVICECOMPONENT_H
#include<Component/Component.h>
namespace GameKeeper
{
    class HttpServiceComponent : public Component
    {
    public:
        HttpServiceComponent() = default;
        ~HttpServiceComponent() override = default;
    protected:
        bool Awake() override;
    };
}

#endif //GAMEKEEPER_HTTPSERVICECOMPONENT_H
