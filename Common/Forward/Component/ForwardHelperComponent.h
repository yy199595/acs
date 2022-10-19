//
// Created by zmhy0073 on 2022/10/18.
//

#ifndef APP_FORWARDHELPERCOMPONENT_H
#define APP_FORWARDHELPERCOMPONENT_H
#include"Client/Message.h"
#include"Component/Component.h"
namespace Sentry
{
    class ForwardHelperComponent final : public Component, public IStart
    {
    public:
        ForwardHelperComponent() = default;
        ~ForwardHelperComponent() = default;
    private:
        bool Start() final;
        bool LateAwake() final;
    public:
        bool AllotLocation(std::string & address);
        bool SendData(std::shared_ptr<Rpc::Packet> message);
        bool SendData(long long userId, std::shared_ptr<Rpc::Packet> message);
        bool SendData(const std::string & target, std::shared_ptr<Rpc::Packet> message);
    private:
        std::vector<std::string> mLocations;
        class InnerNetComponent * mInnerComponent;
        std::unordered_map<std::string, int> mLocationWeights;
    };
}


#endif //APP_FORWARDHELPERCOMPONENT_H
