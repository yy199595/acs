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
        delete session;
	}

	bool TelnetClientComponent::OnListenNewSession(TelnetClientSession * session)
	{
		std::string * message = new std::string("welcom sentry seever\n请输入用户名和密码:");
		session->SendNetMessage(message);
		return true;
	}

	bool TelnetClientComponent::OnReceiveMessage(TelnetClientSession * session, const std::string & message)
    {
        if (message == "exit")
        {
            return false;
        }
        std::string *newMessage = this->mStringPool.New("你说");
        newMessage->append(message + "\n");
        session->SendNetMessage(newMessage);
        return true;
    }

	void TelnetClientComponent::OnSessionError(TelnetClientSession * session, const asio::error_code & err)
	{
	}

	void TelnetClientComponent::OnConnectRemoteAfter(TelnetClientSession * session, const asio::error_code & err)
	{

	}

}