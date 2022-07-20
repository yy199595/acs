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

	class MainRedisComponent final : public RedisComponent, public ILuaRegister
	{
	 public:
		MainRedisComponent() = default;
		~MainRedisComponent() final = default;
	 public:
		bool UnLock(const std::string& key); //分布式锁
		bool Lock(const std::string& key, int timeout = 10);
	 public:
		bool SubscribeChannel(const std::string& channel);
		long long Publish(const std::string& channel, const std::string& message);
		std::shared_ptr<RedisTask> AddRedisTask(std::shared_ptr<RedisRequest> request) final;
		std::shared_ptr<LuaRedisTask> AddLuaRedisTask(std::shared_ptr<RedisRequest> request, lua_State * lua);
	 private:
		bool OnStart() final;
		bool LateAwake() final;
		bool StartSubChannel();
        void OnSecondUpdate(const int tick) final;
		void OnLockTimeout(const std::string& name, int timeout);
		void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) final;
		bool HandlerEvent(const std::string& channel, const std::string& message);
	 private:
		void OnSubscribe(SharedRedisClient redisClient, const std::string& channel, const std::string& message) final;
		void OnCommandReply(SharedRedisClient redisClient, long long id, std::shared_ptr<RedisResponse> response) final;
	 private:
		std::string mRpcAddress;
		TaskComponent* mTaskComponent;
		TimerComponent* mTimerComponent;
		const struct RedisConfig* mConfig;
		SharedRedisClient mSubRedisClient;
		class ServiceRpcComponent* mRpcTaskComponent;
		std::unordered_map<std::string, long long> mLockTimers; //分布式锁的续命定时器
		std::unordered_map<long long, std::shared_ptr<IRpcTask<RedisResponse>>> mTasks;
	};
}