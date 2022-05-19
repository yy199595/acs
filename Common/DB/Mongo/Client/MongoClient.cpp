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

	void MongoClientContext::OnConnect(const asio::error_code& error)
	{

	}

	bool MongoClientContext::OnRecvMessage(const asio::error_code& code, const char* message, size_t size)
	{
		return true;
	}

	void MongoClientContext::OnSendMessage(const asio::error_code& code, std::shared_ptr<ProtoMessage> message)
	{

	}

}