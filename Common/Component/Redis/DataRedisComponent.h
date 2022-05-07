//
// Created by yjz on 2022/4/26.
//

#ifndef _DATAREDISCOMPONENT_H_
#define _DATAREDISCOMPONENT_H_
#include"Component/Component.h"
#include"DB/Redis/RedisInstance.h"
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
	private:
		class TaskComponent * mTaskComponent;
		class TimerComponent * mTimerComponent;
		std::vector<const RedisConfig *> mRedisConfigs;
		std::unordered_map<std::string, std::shared_ptr<RedisInstance>> mRedisInsts;
	};
}

#endif //_DATAREDISCOMPONENT_H_
