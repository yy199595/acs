//
// Created by zmhy0073 on 2022/1/15.
//

#ifndef GAMEKEEPER_REDISCLIENT_H
#define GAMEKEEPER_REDISCLIENT_H
#include"RedisDefine.h"
#include"Tcp/SocketProxy.h"
#include"Tcp/TcpContext.h"
#include"Source/TaskSource.h"
#include"Config/RedisConfig.h"
#include"Coroutine/CoroutineLock.h"

using namespace Tcp;
namespace Sentry
{
    class RedisComponent;
    class TcpRedisClient final : public Tcp::TcpContext
    {
    public:
        TcpRedisClient(std::shared_ptr<SocketProxy> socket, const RedisClientConfig & config, IRpc<RedisResponse> * component);
		~TcpRedisClient();
    public:
        void Send(std::shared_ptr<RedisRequest> command);
		long long Call(std::shared_ptr<RedisRequest> command);
		const RedisClientConfig & GetConfig() { return mConfig;}
		const std::string & GetName() const { return this->mConfig.Name; }
    private:
		bool AuthUser();
		void OnReadComplete();
        void StartPingServer();
        void CloseFreeClient();
        void OnReceiveLine(const asio::error_code &code, std::istream & is, size_t) final;
        void OnReceiveMessage(const asio::error_code &code, std::istream & is, size_t) final;
        std::shared_ptr<RedisResponse> SyncCommand(std::shared_ptr<RedisRequest> command);
        void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
    private:
		std::string mAddress;
        RedisClientConfig mConfig;
		IRpc<RedisResponse> * mComponent;
		std::shared_ptr<asio::steady_timer> mTimer;
        std::shared_ptr<RedisResponse> mCurResponse;
        std::shared_ptr<asio::steady_timer> mCloseTimer;
    };
}
#endif //GAMEKEEPER_REDISCLIENT_H
