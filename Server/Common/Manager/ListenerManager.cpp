#include "ListenerManager.h"
#include<Util/StringHelper.h>
#include<Core/Applocation.h>
#include<Manager/NetSessionManager.h>
namespace SoEasy
{
	bool ListenerManager::StartAccept()
	{
		if (this->mIsAccept == false || this->mBindAcceptor == nullptr)
		{
			return false;
		}

		AsioContext & io = this->mDispatchManager->GetAsioCtx();
		SharedTcpSocket tcpSocket = std::make_shared<AsioTcpSocket>(io);
		this->mBindAcceptor->async_accept(*tcpSocket, [this, tcpSocket](const asio::error_code & code)
			{
				if (!code)
				{
					this->mDispatchManager->Create(tcpSocket);
				}
				this->mIsAccept = true;
			});
		this->mIsAccept = false;
		return true;
	}

	bool ListenerManager::OnInit()
	{
		this->mIsAccept = false;
		this->GetConfig().GetValue("WhiteList", this->mWhiteList);
		SayNoAssertRetFalse_F(this->mDispatchManager = this->GetManager<NetSessionManager>());
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("ListenAddress", this->mListenAddress));
		SayNoAssertRetFalse_F(StringHelper::ParseIpAddress(this->mListenAddress, this->mListenerIp, this->mListenerPort));

		try
		{		
			AsioTcpEndPoint endPoint(asio::ip::tcp::v4(), this->mListenerPort);
			this->mBindAcceptor = new AsioTcpAcceptor(this->mDispatchManager->GetAsioCtx(), endPoint);

			this->mIsAccept = true;
			this->mBindAcceptor->listen();
			SayNoDebugInfo("start listener {" << this->mListenAddress << "}");
			return true;
		}
		catch (const asio::system_error & e)
		{
			SayNoDebugError("start server fail " << e.what());
			return false;
		}

		return true;
	}
}
