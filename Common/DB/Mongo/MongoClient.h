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
		std::string mFullName;
	};
}

namespace Sentry
{
	class MongoRpcComponent;
}

namespace Mongo
{
	class MongoClientContext : public Tcp::TcpContext
	{
	 public:
		MongoClientContext(std::shared_ptr<SocketProxy> scoket, const Mongo::Config & config);
	public:
		void SendMongoCommand(std::shared_ptr<MongoQueryRequest> request);
	private:
		bool StartAuthBySha1();
		void OnReceiveMessage(const asio::error_code &code, std::istream & is) final;
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
	private:
		const Mongo::Config & mConfig;
		asio::streambuf streamBuffer;
		MongoRpcComponent * mMongoComponent;
        std::shared_ptr<MongoQueryResponse> mMongoResponse;
	};
}


#endif //SERVER_MONGOCLIENT_H
