#include"TelnetClientSession.h"
#include<Manager/CommandManager.h>
namespace SoEasy
{
	TelnetClientSession::TelnetClientSession(CommandManager * manager, SharedTcpSocket socket)
		:mAsioContext(manager->GetAsioContext())
	{
		this->mBindSocket = socket;
		this->mSessionManager = manager;
		const std::string ip = socket->remote_endpoint().address().to_string();
		const unsigned short port = socket->remote_endpoint().port();
		this->mAddress = ip + ":" + std::to_string(port);
	}

	void TelnetClientSession::StartRecvMessage()
	{	
		if (this->mBindSocket != nullptr && this->mBindSocket->is_open())
		{
			this->mAsioContext.post(std::bind(&TelnetClientSession::Receive, this));
		}
	}

	void TelnetClientSession::StartWriteMessage(const std::string & message)
	{
		this->mAsioContext.post([this, message]()
		{
			const std::string messageData = message + "\r\n>";
			mBindSocket->async_send(asio::buffer(messageData.c_str(), messageData.size()),
				[this](const asio::error_code & error_code, std::size_t size)
			{
				if (error_code)
				{
					this->Close();
					this->mSessionManager->AddErrorSession(shared_from_this());
				}
			});
		});
	}

	void TelnetClientSession::Close()
	{
		if (this->mBindSocket != nullptr && this->mBindSocket->is_open())
		{
			asio::error_code error_code;
			this->mBindSocket->shutdown(asio::socket_base::shutdown_type::shutdown_both, error_code);
			this->mBindSocket->close(error_code);
		}
	}
	void TelnetClientSession::Receive()
	{
		asio::async_read_until(*this->mBindSocket, this->mRecvMsgBuffer, "\r\n", [this](const asio::error_code & error_code, const std::size_t t)
		{
			if (error_code)
			{
				this->Close();
				this->mSessionManager->AddErrorSession(shared_from_this());
				return;
			}
			std::ostringstream osstream;
			osstream << &this->mRecvMsgBuffer;
			std::string messageData = osstream.str();
			this->mSessionManager->AddReceiveMessage(this->shared_from_this(), messageData);
			this->StartRecvMessage();
		});
	}
}