#pragma once

#include<Component/Component.h>

namespace Sentry
{
    class ServiceProxy;
    class ServiceMgrComponent : public Component
    {
    public:
        ServiceMgrComponent() = default;
        ~ServiceMgrComponent() override = default;
    protected:
        bool Awake() final;
        bool LateAwake() final;
    public:
        std::shared_ptr<ServiceProxy> GetServiceProxy(const std::string & name);
    private:
        std::unordered_map<std::string, std::shared_ptr<ServiceProxy>> mServiceEntityMap;
    };
}