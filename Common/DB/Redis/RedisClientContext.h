//
// Created by zmhy0073 on 2022/1/15.
//

#ifndef GAMEKEEPER_REDISCLIENT_H
#define GAMEKEEPER_REDISCLIENT_H
#include"Network/SocketProxy.h"
#include"Async/TaskSource.h"
#include"DB/Redis/RedisDefine.h"
#include"Network/TcpContext.h"
#include"Coroutine/CoroutineLock.h"

using namespace Tcp;
namespace Sentry
{
	struct RedisConfig;
    class RedisComponent;
    class RedisClientContext final : public Tcp::TcpContext
    {
    public:
        RedisClientContext(std::shared_ptr<SocketProxy> socket, const RedisConfig & config, RedisComponent * component);
		~RedisClientContext();
    public:
        void StartReceiveMessage();
        const RedisConfig & GetConfig() { return mConfig;}
        void SendCommand(std::shared_ptr<RedisRequest> command);
        const std::string & GetName() const { return this->mConfig.Name; }
    private:
		bool AuthUser();
		void OnReadComplete();
		void OnConnect(const asio::error_code &error, int count) final;
        void AddCommandQueue(std::shared_ptr<RedisRequest> command);
        void OnReceiveLine(const asio::error_code &code, std::istream & is) final;
        void OnReceiveMessage(const asio::error_code &code, std::istream & is) final;
        void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
    private:
        const RedisConfig & mConfig;
        RedisComponent * mRedisComponent;
		std::shared_ptr<asio::steady_timer> mTimer;
		std::shared_ptr<RedisResponse> mCurResponse;
		std::list<std::shared_ptr<RedisRequest>> mCommands;
    };
    typedef std::shared_ptr<RedisClientContext> SharedRedisClient;
}
#endif //GAMEKEEPER_REDISCLIENT_H
