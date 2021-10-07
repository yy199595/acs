#include "ListenerComponent.h"

#include<Core/App.h>
#include<Util/StringHelper.h>
#include<NetWork/TcpClientSession.h>
#include<Scene/NetSessionComponent.h>

namespace Sentry
{

    void ListenerComponent::OnNetSystemUpdate(AsioContext &io)
    {
        if (!this->mIsAccept || this->mBindAcceptor == nullptr)
        {
            return;
        }
        SharedTcpSocket tcpSocket = std::make_shared<AsioTcpSocket>(io);
        this->mBindAcceptor->async_accept(*tcpSocket, [this, tcpSocket](const asio::error_code &code) {
            if (!code)
            {
				std::string ip = tcpSocket->remote_endpoint().address().to_string();
				if (this->mWhiteList.empty() || this->mWhiteList.find(ip) != this->mWhiteList.end())
				{
					TcpClientSession *clientSession = this->mDispatchManager->Create(tcpSocket);
					if (clientSession != nullptr)
					{
						const std::string &address = clientSession->GetAddress();
						SayNoDebugInfo("connect new session : [" << address << "]");
					}
				}
				else
				{
					asio::error_code err;
					tcpSocket->close(err);
					SayNoDebugError("Close unauthorized links : [" << ip << "]");
				}          
            }
            this->mIsAccept = true;
        });
        this->mIsAccept = false;
    }

    bool ListenerComponent::Awake()
    {
        this->mIsAccept = false;
		ServerConfig & config = App::Get().GetConfig();
		config.GetValue("WhiteList", this->mWhiteList);
        SayNoAssertRetFalse_F(config.GetValue("ListenAddress", "ip", this->mListenerIp));
        SayNoAssertRetFalse_F(config.GetValue("ListenAddress", "port", this->mListenerPort));
        this->mListenAddress = this->mListenerIp + ":" + std::to_string(this->mListenerPort);
		SayNoAssertRetFalse_F(this->mDispatchManager = this->GetComponent<NetSessionComponent>());
        try
        {
            AsioContext &io = App::Get().GetNetContext();
            auto address = asio::ip::make_address_v4(mListenerIp);
            AsioTcpEndPoint endPoint(address, this->mListenerPort);
            this->mBindAcceptor = new AsioTcpAcceptor(io, endPoint);
            return true;
        }
        catch (const asio::system_error &e)
        {
            SayNoDebugError("start server fail " << e.what());
            return false;
        }
    }

    void ListenerComponent::Start()
    {
        this->mIsAccept = true;
        this->mBindAcceptor->listen();
        SayNoDebugInfo("start listener {" << this->mListenAddress << "}");
    }
}
