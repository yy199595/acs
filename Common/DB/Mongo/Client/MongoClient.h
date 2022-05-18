//
// Created by mac on 2022/5/18.
//

#ifndef SERVER_MONGOCLIENT_H
#define SERVER_MONGOCLIENT_H
#include"asio.hpp"
#include"MongoProto.h"
#include"Network/SocketProxy.h"
#include"Network/TcpContext.h"
namespace Mongo
{
	class MongoClientContext : public Tcp::TcpContext
	{
	 public:
		MongoClientContext(std::shared_ptr<SocketProxy> scoket);
	 protected:
		size_t GetRecvSize() final { return 1024 * 100; }
		void OnError(const asio::error_code& error) final;
		void OnConnect(const asio::error_code& error) final;
		bool OnRecvMessage(const char* message, size_t size) final;
	};
}


#endif //SERVER_MONGOCLIENT_H
