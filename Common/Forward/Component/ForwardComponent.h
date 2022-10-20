//
// Created by zmhy0073 on 2022/10/14.
//

#ifndef APP_FORWARDCOMPONENT_H
#define APP_FORWARDCOMPONENT_H
#include"Client/Message.h"
#include"Client/InnerNetClient.h"
#include"Component/TcpListenerComponent.h"

namespace Sentry
{
    class ForwardComponent : public TcpListenerComponent, public IRpc<Rpc::Packet>
    {
    public:
        ForwardComponent() = default;
        ~ForwardComponent() = default;
    private:
        bool LateAwake() final;
        void StartClose(const std::string &address) final;
        bool OnListen(std::shared_ptr<SocketProxy> socket) final;
        void OnCloseSocket(const std::string &address, XCode code) final;
        void OnMessage(const std::string &address, std::shared_ptr<Rpc::Packet> message) final;
    private:
        XCode OnRequest(std::shared_ptr<Rpc::Packet> message);
		XCode OnResponse(std::shared_ptr<Rpc::Packet> message);
        bool OnAuth(const std::string & address, std::shared_ptr<Rpc::Packet> message);
    private:
        bool IsAuth(const std::string & address) const;
        InnerNetClient * GetClient(const std::string & address);
    private:
        XCode Forward(long long userId, std::shared_ptr<Rpc::Packet> message);
        XCode Forward(const std::string & adress, std::shared_ptr<Rpc::Packet> message);
    public:
        void Send(const std::string & func, const Message * message);
        void Send(const std::string & address, std::shared_ptr<Rpc::Packet> message);
        void Send(const std::string & address, const std::string & func, const Message * message);
    private:
        class LocationComponent * mLocationComponent;
        std::unordered_set<std::string> mAuthClients;
        class ForwardMessageComponent * mMessageComponent;
        std::unordered_map<std::string, std::string> mLocationMap;
        std::unordered_map<std::string, std::shared_ptr<InnerNetClient>> mClients;
    };
}

#endif //APP_FORWARDCOMPONENT_H
