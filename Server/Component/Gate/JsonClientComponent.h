//
// Created by zmhy0073 on 2022/8/1.
//

#ifndef SERVER_JSONCLIENTCOMPONENT_H
#define SERVER_JSONCLIENTCOMPONENT_H
#include"Component/Component.h"
#include"Network/Listener/TcpServerListener.h"
namespace Tcp
{
    class JsonRequest;
    class JsonRpcClient;
}

namespace Sentry
{
    class JsonClientComponent : public Component, public TcpServerListener
    {
    public:
        JsonClientComponent() = default;
    public:
        bool OnListen(std::shared_ptr<SocketProxy> socket) final;
        bool SendToClient(const std::string & address, const std::string & json);
    public:
        void OnRequest(std::shared_ptr<Tcp::JsonRequest> request);
    private:
        std::unordered_map<std::string, std::shared_ptr<Tcp::JsonRpcClient>> mClients;
    };
}


#endif //SERVER_JSONCLIENTCOMPONENT_H
