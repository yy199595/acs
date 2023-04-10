//
// Created by zmhy0073 on 2022/1/15.
//

#ifndef GAMEKEEPER_REDISCLIENT_H
#define GAMEKEEPER_REDISCLIENT_H
#include"RedisDefine.h"
#include"Network/Tcp/SocketProxy.h"
#include"Network/Tcp/TcpContext.h"
#include"Async/Source/TaskSource.h"
#include"Redis/Config/RedisConfig.h"
#include"Async/Coroutine/CoroutineLock.h"
#include"Core/Component/IComponent.h"

namespace Tendo
{
    class RedisComponent;
    class TcpRedisClient final : public Tcp::TcpContext
    {
    public:
        TcpRedisClient(std::shared_ptr<Tcp::SocketProxy> socket,
				const RedisClientConfig & config, IRpc<RedisResponse> * component);
    public:
        void Send(const std::shared_ptr<RedisRequest>& command);
		long long Call(const std::shared_ptr<RedisRequest>& command);
		const RedisClientConfig & GetConfig() { return mConfig;}
		const std::string & GetName() const { return this->mConfig.Name; }
    private:
		bool AuthUser();
		void OnReadComplete();
        void StartPingServer();
        void CloseFreeClient();
        bool InitRedisClient(const std::string& pwd);
        void OnReceiveLine(const asio::error_code &code, std::istream & is, size_t) final;
        void OnReceiveMessage(const asio::error_code &code, std::istream & is, size_t) final;
        std::shared_ptr<RedisResponse> SyncCommand(const std::shared_ptr<RedisRequest>& command);
        void OnSendMessage(const asio::error_code &code, std::shared_ptr<Tcp::ProtoMessage> message) final;
    private:
        size_t mIndex;
		std::string mAddress;
        RedisClientConfig mConfig;
		IRpc<RedisResponse> * mComponent;
		std::shared_ptr<asio::steady_timer> mTimer;
        std::shared_ptr<RedisResponse> mCurResponse;
        std::shared_ptr<asio::steady_timer> mCloseTimer;
    };
}
#endif //GAMEKEEPER_REDISCLIENT_H
