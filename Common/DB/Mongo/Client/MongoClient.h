//
// Created by mac on 2022/5/18.
//

#ifndef SERVER_MONGOCLIENT_H
#define SERVER_MONGOCLIENT_H
#include"asio.hpp"
#include"MongoProto.h"
#include"Network/SocketProxy.h"
#include"Network/TcpContext.h"

using namespace Tcp;
namespace Mongo
{
	class MongoClientContext : public Tcp::TcpContext
	{
	 public:
		MongoClientContext(std::shared_ptr<SocketProxy> scoket);
	 protected:
		void OnConnect(const asio::error_code &error) final;
		bool OnRecvMessage(const asio::error_code &code, const char *message, size_t size) final;
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
	};
}


#endif //SERVER_MONGOCLIENT_H
