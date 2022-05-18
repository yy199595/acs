//
// Created by mac on 2022/5/18.
//

#include "MongoClient.h"

namespace Mongo
{
	MongoClientContext::MongoClientContext(std::shared_ptr<SocketProxy> scoket)
		: Tcp::TcpContext(scoket)
	{

	}

	bool MongoClientContext::OnRecvMessage(const char* message, size_t size)
	{
		return true;
	}

	void MongoClientContext::OnError(const asio::error_code& error)
	{

	}

	void MongoClientContext::OnConnect(const asio::error_code& error)
	{

	}
}