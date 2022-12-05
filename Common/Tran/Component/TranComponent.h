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
    class TranComponent : public TcpListenerComponent, public IRpc<Rpc::Packet>
    {
    public:
        TranComponent() = default;
        ~TranComponent() = default;
    private:
		bool Awake() final;
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
        bool Send(std::shared_ptr<Rpc::Packet> message);
        const ServiceNodeInfo * GetServerInfo(const std::string & address) const;
        bool Send(const std::string & address, std::shared_ptr<Rpc::Packet> message);
    private:
        class LocationComponent * mLocationComponent;
        std::unordered_set<std::string> mAuthClients;
        std::unordered_map<std::string, std::string> mLocationMap;
        std::unordered_map<std::string, std::shared_ptr<InnerNetClient>> mClients;
        std::unordered_map<std::string, std::unique_ptr<ServiceNodeInfo>> mNodeInfos;
    };
}

#endif //APP_FORWARDCOMPONENT_H
