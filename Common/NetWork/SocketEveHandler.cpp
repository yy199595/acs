#include "SocketEveHandler.h"
#include <Core/App.h>
#include <Scene/TcpNetSessionComponent.h>
namespace Sentry
{
	void MainSocketSendHandler::RunHandler(TcpNetSessionComponent * sessionComponent)
	{
		if (!sessionComponent->StartSendMessage(this->mAddress, mMessage))
		{
			SayNoDebugError("send message to " << this->mAddress << " failure");
		}
	}
	
	void MainSocketCloseHandler::RunHandler(TcpNetSessionComponent * sessionComponent)
	{
		if (!sessionComponent->StartClose(mAddress))
		{
			SayNoDebugError("close session " << this->mAddress << " failure");
		}
	}

	void MainSocketConnectHandler::RunHandler(TcpNetSessionComponent * sessionComponent)
	{
		if (!sessionComponent->StartConnect(mAddress, mName))
		{
			SayNoDebugError("connect session " << this->mAddress << " failure");
		}
	}
}

namespace Sentry
{
	void NetSocketConnectHandler::RunHandler(TcpNetProxyComponent * component)
	{
		component->ConnectAfter(mAddress, this->mIsSuccessful);
	}

	void NetNewSocketConnectHandler::RunHandler(TcpNetProxyComponent * component)
	{
		component->NewConnect(mAddress);
	}

	void NetReceiveNewMessageHandler::RunHandler(TcpNetProxyComponent * component)
	{
		component->ReceiveNewMessage(this->mAddress, this->mMessage);
	}

	void NetErrorHandler::RunHandler(TcpNetProxyComponent * component)
	{
		component->SessionError(mAddress);
	}
}
