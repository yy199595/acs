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
    class ForwardComponent : public TcpListenerComponent, public IRpc<Rpc::Data>
    {
    public:
        ForwardComponent() = default;
        ~ForwardComponent() = default;
    private:
        bool LateAwake() final;
        void StartClose(const std::string &address) final;
        bool OnListen(std::shared_ptr<SocketProxy> socket) final;
        void OnCloseSocket(const std::string &address, XCode code) final;
        void OnMessage(const std::string &address, std::shared_ptr<Rpc::Data> message) final;
    private:
        XCode OnRequest(const std::string & address, std::shared_ptr<Rpc::Data> message);
    private:
        bool IsAuth(const std::string & address) const;
        InnerNetClient * GetClient(const std::string & address);
    private:
        std::unordered_set<std::string> mAuthClients;
        std::unordered_map<std::string, std::shared_ptr<InnerNetClient>> mInnerClients;
    };
}

#endif //APP_FORWARDCOMPONENT_H
