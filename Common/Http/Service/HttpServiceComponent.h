//
// Created by zmhy0073 on 2021/11/2.
//

#ifndef GAMEKEEPER_HTTPSERVICECOMPONENT_H
#define GAMEKEEPER_HTTPSERVICECOMPONENT_H
#include "Component/Component.h"
#include "Method/HttpServiceMethod.h"
namespace GameKeeper
{
    class HttpServiceComponent : public Component
    {
    public:
        HttpServiceComponent() = default;
        ~HttpServiceComponent() override;
    public:
        HttpServiceMethod * GetMethod(const std::string & path);
    protected:
        bool Awake() override;

    private:
        std::unordered_map<std::string, HttpServiceMethod *> mMethodMap;
    };
}
#endif //GAMEKEEPER_HTTPSERVICECOMPONENT_H
