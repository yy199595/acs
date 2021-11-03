
#include"TelnetClientSession.h"
#include <Define/CommonDef.h>
namespace GameKeeper
{
	TelnetClientSession::TelnetClientSession(ISocketHandler * hanlder)
		:SessionBase(hanlder, "RemoteTelnetSession"),mDelim("\r\n")
	{

	}

	void TelnetClientSession::OnSessionEnable()
	{
		if (!this->mSocket->is_open())
		{
			return;
		}
		asio::async_read_until(this->GetSocket(), this->mReceiveBuffer, this->mDelim,
			std::bind(&TelnetClientSession::ReadHandler, this, std::placeholders::_1, std::placeholders::_2));
	}

	void TelnetClientSession::ReadHandler(const asio::error_code & err, const size_t size)
	{		
		if (err)
		{			
			this->OnError(err);
		}
		else
		{
			std::istreambuf_iterator<char> eos;
			std::istream is(&this->mReceiveBuffer);
			std::string message(std::istreambuf_iterator<char>(is), eos);

			this->OnReceiveMessage(message.c_str(), message.size() - this->mDelim.size());

			asio::async_read_until(this->GetSocket(), this->mReceiveBuffer, this->mDelim,
				std::bind(&TelnetClientSession::ReadHandler, this, std::placeholders::_1, std::placeholders::_2));
		}		
	}

}

