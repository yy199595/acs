//
// Created by 64658 on 2024/10/10.
//

#pragma once
#include "Mysql/Config/MysqlConfig.h"
#include "Net/Network/Tcp/TcpClient.h"

namespace mysql
{
	class IRequest
	{

	};

	class IResponse
	{

	};
}

namespace mysql
{
	class Client : public tcp::TcpClient
	{
	public:
		explicit Client(tcp::Socket * sock, mysql::MysqlConfig  config);
	public:
		void Send(std::unique_ptr<mysql::IRequest> request);
	private:
		void OnSendMessage() final;
		void OnConnect(bool result, int count) final;
		void OnSendMessage(const Asio::Code &code) final;
	private:
		mysql::MysqlConfig mConfig;
		std::unique_ptr<mysql::IRequest> mRequest;
		std::unique_ptr<mysql::IResponse> mResponse;
	};
}
