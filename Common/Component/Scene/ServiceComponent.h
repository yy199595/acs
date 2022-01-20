#pragma once

#include<Component/Component.h>

namespace Sentry
{
    class ServiceEntity;
    class ServiceComponent : public Component
    {
    public:
        ServiceComponent() = default;
        ~ServiceComponent() override = default;
    protected:
        bool Awake() final;
        bool LateAwake() final;
    public:
        std::shared_ptr<ServiceEntity> GetServiceEntity(const std::string & name);
    private:
        std::unordered_map<std::string, std::shared_ptr<ServiceEntity>> mServiceEntityMap;
    };
}