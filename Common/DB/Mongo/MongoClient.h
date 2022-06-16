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
	class MongoClientContext : public Tcp::TcpContext
	{
	 public:
		MongoClientContext(std::shared_ptr<SocketProxy> scoket);
	public:
		bool StartConnect();
		void SendMongoCommand(std::shared_ptr<Tcp::ProtoMessage> request);
	 protected:
		void OnConnect(const asio::error_code &error) final;
        void OnReceiveHead(const asio::error_code &code, const char *message, size_t size) final;
        void OnReceiveBody(const asio::error_code &code, const char *message, size_t size) final;
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
	private:
        asio::streambuf streamBuffer;
        TaskSource<bool> mWriteTask;
		TaskSource<bool> mConnectTask;
		std::shared_ptr<CoroutineLock> mWriteLock;
		std::shared_ptr<CoroutineLock> mConnectLock;
	};
}


#endif //SERVER_MONGOCLIENT_H
