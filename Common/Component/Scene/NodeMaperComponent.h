//
// Created by zmhy0073 on 2021/10/11.
//

#ifndef GameKeeper_NODEMAPERCOMPONENT_H
#define GameKeeper_NODEMAPERCOMPONENT_H
#include <Component.h>

namespace GameKeeper
{
    class NodeMaperComponent : public Component
    {
    public:
		NodeMaperComponent() = default;
        ~NodeMaperComponent() override = default;
    public:
        void AddService(class ServiceNode * node);
        class ServiceNode * GetService(const std::string & service);
    protected:
        bool Awake() override;

    private:
        std::set<std::string> mServices;
        class ServiceNodeComponent * mServiceNodeComponent;
        std::unordered_map<std::string, int> mServiceMappers;
    };
}


#endif //GameKeeper_NODEMAPERCOMPONENT_H
