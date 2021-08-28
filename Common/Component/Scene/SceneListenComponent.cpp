#include "SceneListenComponent.h"

#include<Core/App.h>
#include<Util/StringHelper.h>
#include<NetWork/TcpClientSession.h>
#include<Component/Scene/SceneSessionComponent.h>

namespace Sentry
{

    void SceneListenComponent::OnNetSystemUpdate(AsioContext &io)
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

    bool SceneListenComponent::Awake()
    {
        this->mIsAccept = false;
		ServerConfig & config = App::Get().GetConfig();
		config.GetValue("WhiteList", this->mWhiteList);
		SayNoAssertRetFalse_F(this->mDispatchManager = Scene::GetComponent<SceneSessionComponent>());
        SayNoAssertRetFalse_F(config.GetValue("ListenAddress", "ip", this->mListenerIp));
        SayNoAssertRetFalse_F(config.GetValue("ListenAddress", "port", this->mListenerPort));
        this->mListenAddress = this->mListenerIp + ":" + std::to_string(this->mListenerPort);
        try
        {
            AsioContext &io = App::Get().GetNetContext();
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
