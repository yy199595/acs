#include "SocketEveHandler.h"
#include <Core/App.h>
#include <Scene/SceneSessionComponent.h>
#include <Scene/SceneNetProxyComponent.h>
namespace Sentry
{
	void MainSocketSendHandler::RunHandler(SceneSessionComponent * sessionComponent)
	{
		if (!sessionComponent->StartSendMessage(mSendMessage))
		{
			const std::string & address = mSendMessage->GetAddress();
			SayNoDebugError("send message to " << address << " failure");
		}
	}
	
	void MainSocketCloseHandler::RunHandler(SceneSessionComponent * sessionComponent)
	{
		if (!sessionComponent->StartClose(mAddress))
		{
			SayNoDebugError("close session " << this->mAddress << " failure");
		}
	}

	void MainSocketConnectHandler::RunHandler(SceneSessionComponent * sessionComponent)
	{
		if (!sessionComponent->StartConnect(mAddress, mName))
		{
			SayNoDebugError("connect session " << this->mAddress << " failure");
		}
	}
}

namespace Sentry
{
	void NetSocketConnectHandler::RunHandler(SceneNetProxyComponent * component)
	{
		if (this->mIsSuccessful)
		{
			component->ConnectSuccessful(mAddress);
		}
		else
		{
			component->ConnectFailure(mAddress);
		}
	}

	void NetNewSocketConnectHandler::RunHandler(SceneNetProxyComponent * component)
	{
		component->NewConnect(mAddress);
	}

	void NetReceiveNewMessageHandler::RunHandler(SceneNetProxyComponent * component)
	{
		component->ReceiveNewMessage(mRecvMessage);
	}

	void NetErrorHandler::RunHandler(SceneNetProxyComponent * component)
	{
		component->SessionError(mAddress);
	}
}
