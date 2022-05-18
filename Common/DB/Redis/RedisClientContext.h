//
// Created by zmhy0073 on 2022/1/15.
//

#ifndef GAMEKEEPER_REDISCLIENT_H
#define GAMEKEEPER_REDISCLIENT_H
#include"Network/SocketProxy.h"
#include"Async/TaskSource.h"
#include"DB/Redis/RedisDefine.h"
#include"Coroutine/CoroutineLock.h"

namespace Sentry
{
	struct RedisConfig;
    class RedisClientContext final : std::enable_shared_from_this<RedisClientContext>
    {
    public:
        RedisClientContext(std::shared_ptr<SocketProxy> socket, const RedisConfig * config);
    public:
		XCode StartConnect();
        bool IsOpen() const { return this->mSocket->IsOpen(); }
		bool IsUse() const { return this->mCommandLock->IsLock(); }
		bool LoadLuaScript(const std::string & path, std::string & key);
		long long GetLastOperatorTime() { return this->mLastOperatorTime;}
    private:
        void OnComplete();
        void StartReceive();
        void OnReceive(const asio::error_code & code, size_t size);
        void ConnectRedis(std::shared_ptr<TaskSource<XCode>> taskSource);
    public:

		XCode Run(std::shared_ptr<RedisRequest> command);
		XCode WaitRedisResponse(std::shared_ptr<RedisResponse> response);
		XCode Run(std::shared_ptr<RedisRequest> command, std::shared_ptr<RedisResponse> response);

    private:
        void OnDecodeHead(std::iostream & readStream);
        void OnDecodeArray(std::iostream & readStream);
        void OnDecodeBinString(std::iostream & readStream);
		int OnReceiveFirstLine(char type, const std::string & lineData);
		void SendCommand(std::shared_ptr<RedisRequest> request, std::shared_ptr<TaskSource<XCode>> taskSource);
	private:
		const RedisConfig * mConfig;
        char mReadTempBuffer[10240];
        long long mLastOperatorTime;
        std::shared_ptr<RedisResponse> mResponse;
		std::shared_ptr<CoroutineLock> mConnectLock; //连接锁
		std::shared_ptr<CoroutineLock> mCommandLock; //命令锁
		std::shared_ptr<TaskSource<XCode>> mReadTaskSource;
    private:
        int mDataSize;
        int mLineCount;
        int mDataCount;
        IAsioThread & mNetworkThread;
        asio::streambuf mRecvDataBuffer;
        asio::streambuf mSendDataBuffer;
        std::shared_ptr<SocketProxy> mSocket;
    };
}
#endif //GAMEKEEPER_REDISCLIENT_H
