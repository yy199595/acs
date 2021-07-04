#include "ListenerManager.h"
#include<Util/StringHelper.h>
#include<Core/Applocation.h>
#include<Manager/NetSessionManager.h>
namespace SoEasy
{
	void ListenerManager::StartAccept()
	{
		if (this->mBindAcceptor != nullptr)
		{
			AsioContext & io = this->mDispatchManager->GetAsioCtx();
			SharedTcpSocket tcpSocket = std::make_shared<AsioTcpSocket>(io);
			this->mBindAcceptor->async_accept(*tcpSocket, [this, tcpSocket](const asio::error_code & code)
				{
					if (!code)
					{
						this->mDispatchManager->Create(tcpSocket);
					}
					AsioContext & io = this->mDispatchManager->GetAsioCtx();
					io.post(BIND_THIS_ACTION_0(ListenerManager::StartAccept));
				});
		}
	}

	bool ListenerManager::OnInit()
	{
		this->GetConfig().GetValue("WhiteList", this->mWhiteList);
		SayNoAssertRetFalse_F(this->mDispatchManager = this->GetManager<NetSessionManager>());
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("ListenAddress", this->mListenAddress));
		SayNoAssertRetFalse_F(StringHelper::ParseIpAddress(this->mListenAddress, this->mListenerIp, this->mListenerPort));

		try
		{
			AsioContext & io = this->mDispatchManager->GetAsioCtx();
			AsioTcpEndPoint endPoint(asio::ip::tcp::v4(), this->mListenerPort);
			this->mBindAcceptor = new AsioTcpAcceptor(io, endPoint);

			this->mBindAcceptor->listen();
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
