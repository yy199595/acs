#pragma once

#include"Util/Guid.h"
#include"Pool/ProtoPool.h"
#include"Json/JsonWriter.h"
#include"Other/ElapsedTimer.h"
#include"RedisBaseComponent.h"
#include"DB/Redis/RedisClientContext.h"
#include"Component/Coroutine/TaskComponent.h"

namespace Sentry
{
	class NetThreadComponent;

	class MainRedisComponent final : public RedisBaseComponent, public IStart
	{
	 public:
		MainRedisComponent() = default;
		~MainRedisComponent() final = default;
	 protected:
		bool LateAwake() final;                //初始化完成之后
		bool OnStart() override;
	 private:
		void StartPubSub();
		bool LoadRedisConfig();
		void CheckRedisClient();
	public:
		bool Lock(const std::string & key, int timeout = 10);
		bool UnLock(const std::string & key);
	 public:
		long long AddCounter(const std::string& key);
		long long SubCounter(const std::string & key);
	 public:
		bool SubscribeChannel(const std::string& channel);
		void GetAllAddress(std::vector<std::string> & chanels);
		long long Publish(const std::string& channel, const std::string& message);
	protected:
		std::shared_ptr<RedisClientContext> GetClient(const std::string &name) final;
	private:
		bool StartSubChannel();
#if __REDIS_DEBUG__
		void StartDebugRedis();
#endif
		void OnLockTimeout(const std::string & name, int timeout);
		bool HandlerEvent(const std::string & channel, const std::string & message);
	private:
		std::string mRpcAddress;
		TaskComponent* mTaskComponent;
		TimerComponent * mTimerComponent;
		const struct RedisConfig * mConfig;
		class RpcHandlerComponent * mRpcComponent;
#if __REDIS_DEBUG__
		std::shared_ptr<RedisClientContext> mDebugClient;
#endif
		std::shared_ptr<RedisClientContext> mRedisClient;
		std::shared_ptr<RedisClientContext> mSubRedisClient;
		std::unordered_map<std::string, long long> mLockTimers; //分布式锁的续命定时器
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