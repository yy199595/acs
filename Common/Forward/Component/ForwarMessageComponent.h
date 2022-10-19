//
// Created by zmhy0073 on 2022/10/19.
//

#ifndef APP_FORWARMESSAGECOMPONENT_H
#define APP_FORWARMESSAGECOMPONENT_H
#include"Client/Message.h"
#include"Component/Component.h"
namespace Sentry
{
    class ForwarMessageComponent final : public Component
    {
    public:
        typedef XCode (ForwarMessageComponent::*MessageCallback)(std::shared_ptr<Rpc::Packet>);
        ForwarMessageComponent() = default;
        ~ForwarMessageComponent() = default;
    public:
        XCode OnAuth(std::shared_ptr<Rpc::Packet> message);
        XCode OnMessage(std::shared_ptr<Rpc::Packet> message);
    private:
        XCode Allot(std::shared_ptr<Rpc::Packet> message);
        XCode Remove(std::shared_ptr<Rpc::Packet> message);
    private:
        bool LateAwake() final;
        bool Add(const std::string & name, MessageCallback && func);
    private:
        class ForwardComponent * mForwardComponent;
        class LocationComponent * mLocationComponent;
        std::unordered_map<std::string, MessageCallback> mHandlers;
    };
}

#endif //APP_FORWARMESSAGECOMPONENT_H
