#include "ListenerManager.h"
#include<Util/StringHelper.h>
#include<Core/Applocation.h>
#include<Manager/NetSessionManager.h>

namespace Sentry
{

    void ListenerManager::OnNetSystemUpdate(AsioContext &io)
    {
        if (this->mIsAccept == false || this->mBindAcceptor == nullptr)
        {
            return;
        }
        SharedTcpSocket tcpSocket = std::make_shared<AsioTcpSocket>(io);
        this->mBindAcceptor->async_accept(*tcpSocket, [this, tcpSocket](const asio::error_code &code) {
            if (!code)
            {
                TcpClientSession *clientSession = this->mDispatchManager->Create(tcpSocket);
                if (clientSession != nullptr)
                {
                    const std::string &address = clientSession->GetAddress();
                    SayNoDebugInfo("connect new session : [" << address << "]");
                }
            }
            this->mIsAccept = true;
        });
        this->mIsAccept = false;
    }

    bool ListenerManager::OnInit()
    {
        this->mIsAccept = false;
        this->GetConfig().GetValue("WhiteList", this->mWhiteList);
        SayNoAssertRetFalse_F(this->mDispatchManager = this->GetManager<NetSessionManager>());

        SayNoAssertRetFalse_F(this->GetConfig().GetValue("ListenAddress", "ip", this->mListenerIp));
        SayNoAssertRetFalse_F(this->GetConfig().GetValue("ListenAddress", "port", this->mListenerPort));
        this->mListenAddress = this->mListenerIp + ":" + std::to_string(this->mListenerPort);
        try
        {
            AsioContext &io = this->GetApp()->GetNetContext();
            AsioTcpEndPoint endPoint(asio::ip::tcp::v4(), this->mListenerPort);
            this->mBindAcceptor = new AsioTcpAcceptor(io, endPoint);

            this->mIsAccept = true;
            this->mBindAcceptor->listen();
            SayNoDebugInfo("start listener {" << this->mListenAddress << "}");
            return true;
        }
        catch (const asio::system_error &e)
        {
            SayNoDebugError("start server fail " << e.what());
            return false;
        }
    }
}
