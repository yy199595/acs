#include "SocketEveHandler.h"
#include <Core/App.h>
#include <Scene/NetSessionComponent.h>
#include <Scene/NetProxyComponent.h>
namespace Sentry
{
	void MainSocketSendHandler::RunHandler(NetSessionComponent * sessionComponent)
	{
		if (!sessionComponent->StartSendMessage(mSendMessage))
		{
			const std::string & address = mSendMessage->GetAddress();
			SayNoDebugError("send message to " << address << " failure");
		}
	}
	
	void MainSocketCloseHandler::RunHandler(NetSessionComponent * sessionComponent)
	{
		if (!sessionComponent->StartClose(mAddress))
		{
			SayNoDebugError("close session " << this->mAddress << " failure");
		}
	}

	void MainSocketConnectHandler::RunHandler(NetSessionComponent * sessionComponent)
	{
		if (!sessionComponent->StartConnect(mAddress, mName))
		{
			SayNoDebugError("connect session " << this->mAddress << " failure");
		}
	}
}

namespace Sentry
{
	void NetSocketConnectHandler::RunHandler(NetProxyComponent * component)
	{
		component->ConnectAfter(mAddress, this->mIsSuccessful);
	}

	void NetNewSocketConnectHandler::RunHandler(NetProxyComponent * component)
	{
		component->NewConnect(mAddress);
	}

	void NetReceiveNewMessageHandler::RunHandler(NetProxyComponent * component)
	{
		component->ReceiveNewMessage(mRecvMessage);
	}

	void NetErrorHandler::RunHandler(NetProxyComponent * component)
	{
		component->SessionError(mAddress);
	}
}
