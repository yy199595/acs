//
// Created by yy on 2024/5/12.
//

#ifndef APP_DELAYQUEUECOMPONENT_H
#define APP_DELAYQUEUECOMPONENT_H
#include "Entity/Component/Component.h"
namespace acs
{
	class DelayQueueComponent : public Component
	{
	public:
		DelayQueueComponent();
		~DelayQueueComponent() = default;
	public:
		std::vector<std::string> List(const std::string & key);
		int Del(const std::string & key, const std::string & value);
		int Del(const std::string & key, const std::vector<std::string> & members);
	public:
		int Add(const std::string & key, const std::string & value, int second);
		int Add(const std::string & key, const std::vector<std::string> & value, int second);
	private:
		bool LateAwake() final;
	private:
		class RedisComponent * mRedis;
	};
}


#endif //APP_DELAYQUEUECOMPONENT_H
