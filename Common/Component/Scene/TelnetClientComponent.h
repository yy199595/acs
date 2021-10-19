#pragma once
#include "Component.h"
#include <NetWork/TelnetClientSession.h>
namespace Sentry
{
	class TelnetClientComponent : public Component, public ScoketHandler<TelnetClientSession>
	{
	public:
		TelnetClientComponent();
		~TelnetClientComponent();
	public:
		bool Awake() override;
		SessionBase * CreateSocket() override;
	protected:		
		 void OnCloseSession(TelnetClientSession * session) override;
		 bool OnListenNewSession(TelnetClientSession * session) override;
		 bool OnReceiveMessage(TelnetClientSession * session, SharedMessage message) override;
		 void OnSessionError(TelnetClientSession * session, const asio::error_code & err) override;
		 void OnConnectRemoteAfter(TelnetClientSession * session, const asio::error_code & err) override;
		 void OnSendMessageAfter(TelnetClientSession * session, SharedMessage message, const asio::error_code & err) override;
	};
}

