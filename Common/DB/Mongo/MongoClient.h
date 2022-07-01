//
// Created by mac on 2022/5/18.
//

#ifndef SERVER_MONGOCLIENT_H
#define SERVER_MONGOCLIENT_H
#include"asio.hpp"
#include"MongoProto.h"
#include"Async/TaskSource.h"
#include"Network/SocketProxy.h"
#include"Network/TcpContext.h"
#include"Coroutine/CoroutineLock.h"
using namespace Tcp;
using namespace Sentry;

namespace Mongo
{
	struct Config
	{
		int mMaxCount;
		std::string mDb;
		std::string mIp;
		std::string mUser;
		std::string mPasswd;
		unsigned short mPort;
	};
}

namespace Sentry
{
	class MongoComponent;
}

namespace Mongo
{
	class MongoClientContext : public Tcp::TcpContext
	{
	 public:
		MongoClientContext(std::shared_ptr<SocketProxy> scoket,
				const Mongo::Config & config, MongoComponent * component, int index);
	public:
		void PushMongoCommand(std::shared_ptr<Tcp::ProtoMessage> request);
	private:
		std::shared_ptr<Mongo::MongoQueryRequest> AuthRequest1();
		void OnConnect(const asio::error_code &error, int count) final;
		void PushCommand(std::shared_ptr<Tcp::ProtoMessage> request);
		void OnReceiveMessage(const asio::error_code &code, asio::streambuf &buffer) final;
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
	private:
		int mIndex;
		const Mongo::Config & mConfig;
		asio::streambuf streamBuffer;
		MongoComponent * mMongoComponent;
		MongoQueryResponse mMongoResponse;
		std::shared_ptr<asio::steady_timer> mTimer;
		std::list<std::shared_ptr<Tcp::ProtoMessage>> mCommands;
	};
}


#endif //SERVER_MONGOCLIENT_H
