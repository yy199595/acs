#pragma once
#include "TelnetClientSession.h"
#include <Component/Component.h>
namespace Sentry
{
    class TelnetClientComponent : public Component, public SocketHandler<TelnetClientSession>
    {
    public:
        TelnetClientComponent();

        ~TelnetClientComponent();

    public:
        bool Awake() override;

        SessionBase *CreateSocket() override;

    protected:
        void OnCloseSession(TelnetClientSession *session) override;

        void OnListenNewSession(TelnetClientSession *session, const asio::error_code &err) override;

        void OnSessionError(TelnetClientSession *session, const asio::error_code &err) override;

        bool OnReceiveMessage(TelnetClientSession *session, const std::string &message) override;

        void OnConnectRemoteAfter(TelnetClientSession *session, const asio::error_code &err) override;
    };
}

