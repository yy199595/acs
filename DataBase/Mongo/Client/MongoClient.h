//
// Created by mac on 2022/5/18.
//

#ifndef SERVER_MONGOCLIENT_H
#define SERVER_MONGOCLIENT_H
#include"asio.hpp"
#include"MongoProto.h"
#include"Tcp/TcpContext.h"
#include"Tcp/SocketProxy.h"
#include"Source/TaskSource.h"
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
	class MongoDBComponent;
}

namespace Mongo
{
	class TcpMongoClient : public Tcp::TcpContext
	{
	 public:
		TcpMongoClient(std::shared_ptr<SocketProxy> scoket, const Mongo::Config & config);
	public:
		void SendMongoCommand(std::shared_ptr<CommandRequest> request);
	private:
		bool StartAuthBySha1();
        bool Auth(const std::string & user, const std::string & db, const std::string & pwd);
		void OnReceiveMessage(const asio::error_code &code, std::istream & is, size_t) final;
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
		std::shared_ptr<CommandResponse> SyncSendMongoCommand(std::shared_ptr<CommandRequest> request);
	private:
		const Mongo::Config & mConfig;
		asio::streambuf streamBuffer;
		MongoDBComponent * mMongoComponent;
        std::shared_ptr<CommandResponse> mMongoResponse;
	};
}


#endif //SERVER_MONGOCLIENT_H
