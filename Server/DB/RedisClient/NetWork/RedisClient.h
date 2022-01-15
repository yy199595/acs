//
// Created by zmhy0073 on 2022/1/15.
//

#ifndef GAMEKEEPER_REDISCLIENT_H
#define GAMEKEEPER_REDISCLIENT_H
#include"SocketProxy.h"
#include"Async/TaskSource.h"
namespace GameKeeper
{
    enum class RedisRespType
    {
        REDIS_NONE,
        REDIS_STRING,
        REDIS_NUMBER,
        REDIS_ARRAY,
        REDIS_STRING_ARRAY,
        REDIS_ERROR
    };
    class RedisClient
    {
    public:
        RedisClient(std::shared_ptr<SocketProxy> socket);
    public:
        std::shared_ptr<TaskSource<bool>> ConnectAsync(const std::string & ip, unsigned short port);

    private:
        void StartReceive();
        void OnReceiveComplete();
        void OnReceive(const asio::error_code & code, size_t size);
        int OnReceiveFirstLine(char type, const std::string & lineData);
        void ConnectRedis(std::shared_ptr<TaskSource<bool>> taskSource);

    private:
        void SendCommand(const std::string & data);
    public:
        int mLineCount;
        int mDataCount;
        std::string mIp;
        unsigned short mPort;
        std::atomic_bool mIsOpen;
        RedisRespType mRedisRespType;
        NetWorkThread & mNetworkThread;
        asio::streambuf mMessageBuffer;
        std::vector<std::string> mLineArray;
        std::shared_ptr<SocketProxy> mSocket;
    };
}
#endif //GAMEKEEPER_REDISCLIENT_H
