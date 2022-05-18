//
// Created by yjz on 2022/4/26.
//

#ifndef _DATAREDISCOMPONENT_H_
#define _DATAREDISCOMPONENT_H_
#include"RedisBaseComponent.h"
#include"DB/Redis/RedisNode.h"
namespace Sentry
{
	struct RedisConfig;
	class DataRedisComponent final : public RedisBaseComponent, public IStart, public ISecondUpdate
	{
	 public:
		DataRedisComponent() = default;
		~DataRedisComponent() = default;
	protected:
		bool OnStart() final;
		bool LateAwake() final;
		void OnSecondUpdate() final;
	public:
		std::shared_ptr<RedisClientContext> GetClient(const std::string & name) final;
	private:
		size_t mTick;
		class TaskComponent * mTaskComponent;
		class TimerComponent * mTimerComponent;
		std::vector<const RedisConfig *> mRedisConfigs;
		std::unordered_map<std::string, std::shared_ptr<RedisNode>> mRedisNodes;
	};
}

#endif //_DATAREDISCOMPONENT_H_
