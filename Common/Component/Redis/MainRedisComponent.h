#pragma once

#include"Util/Guid.h"
#include"Pool/ProtoPool.h"
#include"Json/JsonWriter.h"
#include"Other/ElapsedTimer.h"
#include"Component/Component.h"
#include"DB/Redis/RedisClientContext.h"
#include"Component/Coroutine/TaskComponent.h"

namespace Sentry
{
	class ThreadPoolComponent;

	class MainRedisComponent final : public Component, public IStart
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
		bool CallLua(const std::string & fullName, Json::Writer & request);
		bool CallLua(const std::string & fullName, Json::Writer & request, std::shared_ptr<Json::Reader> response);
	 public:
		long long AddCounter(const std::string& key);
		long long SubCounter(const std::string & key);
	 public:
		bool SubscribeChannel(const std::string& channel);
		void GetAllAddress(std::vector<std::string> & chanels);
		long long Publish(const std::string& channel, const std::string& message);
	private:
		bool SubEvent();
		std::shared_ptr<RedisClientContext> MakeRedisClient();
		void OnLockTimeout(const std::string & name, int timeout);
		bool GetLuaScript(const std::string& file, std::string& command);
		bool HandlerEvent(const std::string & channel, const std::string & message);
	private:
		std::string mResString;
		std::string mRpcAddress;
		TaskComponent* mTaskComponent;
		TimerComponent * mTimerComponent;
		const struct RedisConfig * mConfig;
		class RpcHandlerComponent * mRpcComponent;
		std::shared_ptr<RedisClientContext> mRedisClient;
		std::shared_ptr<RedisClientContext> mSubRedisClient;
		std::unordered_map<std::string, std::string> mLuaCommandMap;
		std::unordered_map<std::string, long long> mLockTimers; //分布式锁的续命定时器
		std::unordered_map<long long, std::shared_ptr<TaskSource<std::shared_ptr<Json::Reader>>>> mPublishTasks;
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