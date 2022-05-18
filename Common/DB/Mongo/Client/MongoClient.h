//
// Created by mac on 2022/5/18.
//

#ifndef SERVER_MONGOCLIENT_H
#define SERVER_MONGOCLIENT_H
#include"asio.hpp"
#include"MongoProto.h"
#include"Network/SocketProxy.h"

namespace Sentry
{
	class MongoClientContext : public std::enable_shared_from_this<MongoClientContext>
	{
	public:
		MongoClientContext(std::shared_ptr<SocketProxy> scoket);

	public:

	};
}


#endif //SERVER_MONGOCLIENT_H
