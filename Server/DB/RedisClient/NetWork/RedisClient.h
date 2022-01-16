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
        REDIS_BIN_STRING,
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
        void ConnectRedis(std::shared_ptr<TaskSource<bool>> taskSource);
    private:
        void SendCommand();
        void SendCommand(std::shared_ptr<std::list<std::string>> data);

    private:
        void OnDecodeHead(std::iostream & readStream);
        void OnDecodeArray(std::iostream & readStream);
        void OnDecodeBinString(std::iostream & readStream);
        int OnReceiveFirstLine(char type, const std::string & lineData);
    private:
        int mDataSize;
        char * mTempReadBuffer;
        char mReadBuffer[10240];
    public:
        bool mIsSend;
        int mLineCount;
        int mDataCount;
        std::string mIp;
        unsigned short mPort;
        std::atomic_bool mIsOpen;
        RedisRespType mRedisRespType;
        NetWorkThread & mNetworkThread;
        asio::streambuf mMessageBuffer;
        asio::streambuf mSendDataBuffer;
        std::vector<std::string> mLineArray;
        std::shared_ptr<SocketProxy> mSocket;
        std::queue<std::shared_ptr<std::list<std::string>>> mSendQueue;
    };
}
#endif //GAMEKEEPER_REDISCLIENT_H
