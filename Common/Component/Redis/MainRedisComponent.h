#pragma once

#include"Util/Guid.h"
#include"Json/JsonWriter.h"
#include"Other/ElapsedTimer.h"
#include"RedisComponent.h"
#include"DB/Redis/RedisClientContext.h"
#include"Component/Coroutine/TaskComponent.h"

namespace Sentry
{
	class NetThreadComponent;

    class MainRedisComponent final : public RedisComponent
	{
	 public:
		MainRedisComponent() = default;
		~MainRedisComponent() final = default;
	 protected:
        bool OnStart() final;
        bool LateAwake() final;                //初始化完成之后
	 private:
		bool LoadRedisConfig();
	public:
		bool Lock(const std::string & key, int timeout = 10);
		bool UnLock(const std::string & key);
	 public:
		bool SubscribeChannel(const std::string& channel);
		long long Publish(const std::string& channel, const std::string& message);
	private:
		bool StartSubChannel();
		void OnLockTimeout(const std::string & name, int timeout);
		bool HandlerEvent(const std::string & channel, const std::string & message);

    private:
        void OnCommandReply(std::shared_ptr<RedisResponse> response) final;
        bool AddRedisTask(std::shared_ptr<IRpcTask<RedisResponse>> task) final;
        void OnSubscribe(const std::string &channel, const std::string &message) final;
	private:
		std::string mRpcAddress;
		TaskComponent* mTaskComponent;
		TimerComponent * mTimerComponent;
		const struct RedisConfig * mConfig;
        SharedRedisClient mSubRedisClient;
        class RpcHandlerComponent * mRpcComponent;
		std::unordered_map<std::string, long long> mLockTimers; //分布式锁的续命定时器
        std::unordered_map<long long,std::shared_ptr<IRpcTask<RedisResponse>>> mTasks;
	};
}

namespace Sentry
{
	class AutoRedisLock
	{
	public:
		AutoRedisLock(MainRedisComponent * component, const std::string & key);
		AutoRedisLock(const AutoRedisLock &) = delete;
		~AutoRedisLock();
	public:
		bool IsLock() { return this->mIsLock;}
	private:
		bool mIsLock;
		const std::string mKey;
		MainRedisComponent * mRedisComponent;
	};
}