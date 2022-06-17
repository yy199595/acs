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
        XCode StartConnectAsync();
        void StartReceiveMessage();
        const RedisConfig & GetConfig() { return mConfig;}
        void SendCommand(std::shared_ptr<RedisRequest> command);
        const std::string & GetName() const { return this->mConfig.Name; }
    private:
        void OnReadComplete();
		void OnConnect(const asio::error_code &error) final;
        void OnReceive(const asio::error_code & code, size_t size);
        void AddCommandQueue(std::shared_ptr<RedisRequest> command);
        void OnReceiveLine(const asio::error_code &code, const std::string &buffer) final;
        void OnReceiveMessage(const asio::error_code &code, const std::string &buffer) final;
        void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
    private:
        const RedisConfig & mConfig;
        asio::streambuf mRecvDataBuffer;
        RedisComponent * mRedisComponent;
        TaskSource<XCode> mConnectTaskSource;
        std::shared_ptr<RedisResponse> mCurResponse;
        std::shared_ptr<CoroutineLock> mConnectLock; //连接锁
        std::queue<std::shared_ptr<RedisRequest>> mCommands;
    };
    typedef std::shared_ptr<RedisClientContext> SharedRedisClient;
}
#endif //GAMEKEEPER_REDISCLIENT_H
