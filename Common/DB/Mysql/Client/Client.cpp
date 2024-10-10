//
// Created by 64658 on 2024/10/10.
//

#include "Client.h"

#include <utility>

namespace mysql
{
	Client::Client(tcp::Socket * sock, mysql::MysqlConfig config)
		: tcp::TcpClient(sock, 1024 * 1024), mConfig(std::move(config))
	{

	}

	void Client::Send(std::unique_ptr<mysql::IRequest> request)
	{

	}

	void Client::OnSendMessage()
	{

	}

	void Client::OnSendMessage(const Asio::Code& code)
	{
		this->Connect();
	}

	void Client::OnConnect(bool result, int count)
	{
		if(!result)
		{
			return;
		}
		this->mSocket->SetOption(tcp::OptionType::NoDelay, true);
		this->mSocket->SetOption(tcp::OptionType::KeepAlive, true);
	}
}