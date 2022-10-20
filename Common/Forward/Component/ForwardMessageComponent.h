//
// Created by zmhy0073 on 2022/10/19.
//

#ifndef APP_FORWARDMESSAGECOMPONENT_H
#define APP_FORWARDMESSAGECOMPONENT_H
#include"Client/Message.h"
#include"Component/Component.h"
namespace Sentry
{
    class ForwardMessageComponent final : public Component
    {
    public:
        typedef XCode (ForwardMessageComponent::*MessageCallback)(std::shared_ptr<Rpc::Packet>);
        ForwardMessageComponent() = default;
        ~ForwardMessageComponent() = default;
    public:
        XCode OnMessage(std::shared_ptr<Rpc::Packet> message);
        const ServiceNodeInfo * OnAuth(std::shared_ptr<Rpc::Packet> message);
    private:
        XCode Allot(std::shared_ptr<Rpc::Packet> message);
        XCode Remove(std::shared_ptr<Rpc::Packet> message);
        XCode Publish(std::shared_ptr<Rpc::Packet> message); //发布
        XCode Subscribe(std::shared_ptr<Rpc::Packet> message); //订阅
        XCode UnSubscribe(std::shared_ptr<Rpc::Packet> message); //取消订阅
    private:
        bool LateAwake() final;
        bool Add(const std::string & name, MessageCallback && func);
    private:
        class ForwardComponent * mForwardComponent;
        class LocationComponent * mLocationComponent;
        std::unordered_map<std::string, MessageCallback> mHandlers;
        std::unordered_map<std::string, std::set<std::string>> mSubMessages;
        std::unordered_map<std::string, std::unique_ptr<ServiceNodeInfo>> mNodeInfos;
    };
}

#endif //APP_FORWARDMESSAGECOMPONENT_H
