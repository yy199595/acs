//
// Created by mac on 2022/5/18.
//

#ifndef SERVER_MONGOCLIENT_H
#define SERVER_MONGOCLIENT_H
#include"MongoProto.h"
#include"Tcp/TcpContext.h"
#include"Tcp/SocketProxy.h"
#include"Source/TaskSource.h"
#include"Coroutine/CoroutineLock.h"
using namespace Tcp;
using namespace Sentry;

namespace Sentry
{
	class MongoDBComponent;
}

namespace Mongo
{
	class TcpMongoClient : public Tcp::TcpContext
	{
	 public:
		TcpMongoClient(std::shared_ptr<SocketProxy> socket, IRpc<CommandResponse> * component);
	public:
        void Stop();
		void SendMongoCommand(std::shared_ptr<CommandRequest> request);
	private:
		bool StartAuthBySha1();
        bool Auth(const std::string & user, const std::string & db, const std::string & pwd);
		void OnReceiveMessage(const asio::error_code &code, std::istream & is, size_t) final;
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
		std::shared_ptr<CommandResponse> SyncSendMongoCommand(std::shared_ptr<CommandRequest> request);
	private:
        size_t mIndex;
        std::string mAddress;
		asio::streambuf streamBuffer;
		IRpc<CommandResponse> * mComponent;
        std::shared_ptr<CommandResponse> mMongoResponse;
	};
}


#endif //SERVER_MONGOCLIENT_H
