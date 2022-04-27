//
// Created by yjz on 2022/4/26.
//

#ifndef _DATAREDISCOMPONENT_H_
#define _DATAREDISCOMPONENT_H_
#include"Component/Component.h"
#include"DB/Redis/RedisClient.h"
namespace Sentry
{
	struct RedisConfig;
	class DataRedisComponent final : public Component, public IStart
	{
	 public:
		DataRedisComponent() = default;
		~DataRedisComponent() = default;
	 public:
		bool OnStart() final;
		bool LateAwake() final;
	 public:
		template<typename ... Args>
		std::shared_ptr<RedisResponse> Run(const std::string & cmd, Args && ... args);
	 private:
		std::shared_ptr<RedisClient> MakeRedisClient(const RedisConfig * config);
	private:
		class TaskComponent * mTaskComponent;
		class TimerComponent * mTimerComponent;
		std::vector<const RedisConfig *> mRedisConfigs;
		std::queue<std::shared_ptr<RedisClient>> mFreeClients;
	};

	template<typename... Args>
	std::shared_ptr<RedisResponse> DataRedisComponent::Run(const string& cmd, Args&& ... args)
	{
		assert(!this->mFreeClients.empty());
		std::shared_ptr<RedisClient> redisClient = this->mFreeClients.front();
		this->mFreeClients.pop();
		this->mFreeClients.push(redisClient);
		return redisClient->Run(cmd, std::forward<Args>(args) ...);
	}
}

#endif //_DATAREDISCOMPONENT_H_
