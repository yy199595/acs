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
    class RedisClient : std::enable_shared_from_this<RedisClient>
    {
    public:
        RedisClient(std::shared_ptr<SocketProxy> socket, const RedisConfig * config);
    public:
		bool StartConnect();
        bool IsOpen() const { return this->mIsOpen; }
		bool LoadLuaScript(const std::string & path, std::string & key);
		long long GetLastOperatorTime() { return this->mLastOperatorTime;}
    private:
        void OnComplete();
        void StartReceive();
        void OnReceive(const asio::error_code & code, size_t size);
        void ConnectRedis(std::shared_ptr<TaskSource<bool>> taskSource);

    public:
        std::shared_ptr<RedisResponse> WaitRedisMessage();
		std::shared_ptr<RedisResponse> Run(std::shared_ptr<RedisRequest> command);

		template<typename ... Args>
		std::shared_ptr<RedisResponse> Run(const std::string & cmd, Args&& ...args);

		template<typename ... Args>
		std::shared_ptr<RedisResponse> Call(const std::string & key, const std::string & method,  Args&& ...args);
    private:
        void OnDecodeHead(std::iostream & readStream);
        void OnDecodeArray(std::iostream & readStream);
        void OnClientError(const asio::error_code & code);
        void OnDecodeBinString(std::iostream & readStream);
        int OnReceiveFirstLine(char type, const std::string & lineData);
    private:
		const RedisConfig * mConfig;
        char mReadTempBuffer[10240];
        long long mLastOperatorTime;
		std::shared_ptr<CoroutineLock> mLock;
        std::shared_ptr<RedisResponse> mResponse;
        TaskSourceShared<RedisResponse> mRespTaskSource;
    private:
        int mDataSize;
        int mLineCount;
        int mDataCount;
        std::atomic_bool mIsOpen;
        IAsioThread & mNetworkThread;
        asio::streambuf mRecvDataBuffer;
        asio::streambuf mSendDataBuffer;
        std::shared_ptr<SocketProxy> mSocket;
    };

	template<typename... Args>
	std::shared_ptr<RedisResponse> RedisClient::Run(const string& cmd, Args &&... args)
	{
		std::shared_ptr<RedisRequest> request
				= std::make_shared<RedisRequest>(cmd);
		request->InitParameter(std::forward<Args>(args)...);
		return this->Run(request);
	}

	template<typename... Args>
	std::shared_ptr<RedisResponse> RedisClient::Call(const string& key, const std::string & method, Args &&... args)
	{
		int size = sizeof ...(Args) + 1;
		return this->Run("EVALSHA", key, size, method, std::forward<Args>(args)...);
	}
}
#endif //GAMEKEEPER_REDISCLIENT_H
