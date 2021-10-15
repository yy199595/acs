#include "TcpServerComponent.h"

#include<Core/App.h>
#include<Util/StringHelper.h>
#include<NetWork/TcpClientSession.h>
#include<Scene/TcpNetSessionComponent.h>

namespace Sentry
{

    void TcpServerComponent::OnTcpContextUpdate(AsioContext &io)
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

    bool TcpServerComponent::Awake()
    {
        this->mIsAccept = false;
        this->mMaxConnectCount = 10000;
		ServerConfig & config = App::Get().GetConfig();
		config.GetValue("WhiteList", this->mWhiteList);
        config.GetValue("ListenAddress", "maxConnect", this->mListenerIp);
        SayNoAssertRetFalse_F(config.GetValue("ListenAddress", "ip", this->mListenerIp));
        SayNoAssertRetFalse_F(config.GetValue("ListenAddress", "port", this->mListenerPort));
        this->mListenAddress = this->mListenerIp + ":" + std::to_string(this->mListenerPort);
		SayNoAssertRetFalse_F(this->mDispatchManager = this->GetComponent<TcpNetSessionComponent>());
        try
        {
            AsioContext &io = App::Get().GetTcpContext();
            AsioTcpEndPoint endPoint(asio::ip::tcp::v4(), this->mListenerPort);
            return (this->mBindAcceptor = new AsioTcpAcceptor(io, endPoint))->is_open();
        }
        catch (const asio::system_error &e)
        {
            SayNoDebugError("start server fail " << e.what());
            return false;
        }
    }

    void TcpServerComponent::Start()
    {
        asio::error_code err;
        this->mIsAccept = true;
        this->mBindAcceptor->listen(this->mMaxConnectCount, err);
        if(!err)
        {
            SayNoDebugInfo("start listener {"
                                   << this->mBindAcceptor->local_endpoint().address().to_string()
                                   << ":" << this->mBindAcceptor->local_endpoint().port()
                                   << "  max count = " << this->mMaxConnectCount << "}");
        }
        else
        {
            SayNoDebugError(err.message());
            App::Get().Stop();
        }
    }
}
