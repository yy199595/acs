#include "TelnetClientComponent.h"

namespace Sentry
{
	TelnetClientComponent::TelnetClientComponent()
	{

	}

	TelnetClientComponent::~TelnetClientComponent()
	{

	}

	bool TelnetClientComponent::Awake()
	{
		return true;
	}

	Sentry::SessionBase * TelnetClientComponent::CreateSocket()
	{
		return new TelnetClientSession(this);
	}

	void TelnetClientComponent::OnCloseSession(TelnetClientSession * session)
	{

	}

	bool TelnetClientComponent::OnListenNewSession(TelnetClientSession * session)
	{
		SharedMessage message = make_shared<std::string>("ÇëÊäÈëÃÜÂë");
		message->append("\n");
		session->SendNetMessage(message);
		return true;
	}

	bool TelnetClientComponent::OnReceiveMessage(TelnetClientSession * session, SharedMessage message)
	{
		SayNoDebugWarning(*message);

		message->append("\n");
		session->SendNetMessage(message);
		return true;
	}

	void TelnetClientComponent::OnSessionError(TelnetClientSession * session, const asio::error_code & err)
	{
	}

	void TelnetClientComponent::OnConnectRemoteAfter(TelnetClientSession * session, const asio::error_code & err)
	{

	}

	void TelnetClientComponent::OnSendMessageAfter(TelnetClientSession * session, SharedMessage message, const asio::error_code & err)
	{

	}

}