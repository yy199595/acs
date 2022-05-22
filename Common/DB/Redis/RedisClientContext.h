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
    class RedisClientContext final : public Tcp::TcpContext
    {
    public:
        RedisClientContext(std::shared_ptr<SocketProxy> socket, const RedisConfig * config);
		~RedisClientContext();
    public:
		XCode StartConnect();
		bool IsUse() const { return this->mCommandLock->IsLock(); }
		bool LoadLuaScript(const std::string & path, std::string & key);
    private:
        void OnComplete();
        void StartReceive();
        void OnReceive(const asio::error_code & code, size_t size);
	protected:
		void OnConnect(const asio::error_code &error) final;
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
	public:
		XCode Run(std::shared_ptr<RedisRequest> command);
		XCode WaitRedisResponse(std::shared_ptr<RedisResponse> response);
		XCode Run(std::shared_ptr<RedisRequest> command, std::shared_ptr<RedisResponse> response);
    private:
        void OnDecodeHead(std::iostream & readStream);
        void OnDecodeArray(std::iostream & readStream);
        void OnDecodeBinString(std::iostream & readStream);
		int OnReceiveFirstLine(char type, const std::string & lineData);
	private:
		const RedisConfig * mConfig;
        char mReadTempBuffer[10240];
		TaskSource<XCode> mReadTaskSource;
		TaskSource<XCode> mSendTaskSource;
		TaskSource<XCode> mConnectTaskSource;
        std::shared_ptr<RedisResponse> mResponse;
		std::shared_ptr<CoroutineLock> mConnectLock; //连接锁
		std::shared_ptr<CoroutineLock> mCommandLock; //命令锁
    private:
        int mDataSize;
        int mLineCount;
        int mDataCount;
        asio::streambuf mRecvDataBuffer;
        asio::streambuf mSendDataBuffer;
    };
}
#endif //GAMEKEEPER_REDISCLIENT_H
