//
// Created by yjz on 2022/4/26.
//

#ifndef _DATAREDISCOMPONENT_H_
#define _DATAREDISCOMPONENT_H_
#include"Component/Component.h"
#include"DB/Redis/RedisClient.h"
namespace Sentry
{
	class DataRedisComponent final : public Component, public IStart
	{
	 public:
		DataRedisComponent() = default;
		~DataRedisComponent() = default;
	 public:
		bool OnStart() final;
		bool LateAwake() final;
	 public:
		std::shared_ptr<RedisResponse> Invoke(const std::string & cmd, std::vector<std::string> & parameter);
	 private:
		std::shared_ptr<RedisClient> MakeRedisClient();
	 private:
		RedisConfig mConfig;
		class TaskComponent * mTaskComponent;
		class TimerComponent * mTimerComponent;
		std::queue<std::shared_ptr<RedisClient>> mFreeClients;
	};
}

#endif //_DATAREDISCOMPONENT_H_
