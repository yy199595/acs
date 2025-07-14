//
// Created by yy on 2024/5/12.
//

#ifndef APP_DELAYMQCOMPONENT_H
#define APP_DELAYMQCOMPONENT_H
#include "Entity/Component/Component.h"

namespace redis
{
	struct DelayResult
	{
	public:
		size_t count = 0;
		std::unique_ptr<std::string[]> list;
	};
}

namespace acs
{
	// redis延时队列
	class DelayMQComponent final : public Component
	{
	public:
		DelayMQComponent();
		~DelayMQComponent() final = default;
	public:
		std::vector<std::string> List(const std::string & key);
		bool Del(const std::string & key, const std::string & value);
		bool Del(const std::string & key, const std::vector<std::string> & members);
	public:
		bool Add(const std::string & key, const std::string & value, int second);
		bool Add(const std::string & key, const std::vector<std::string> & value, int second);
	private:
		bool LateAwake() final;
	private:
		class RedisComponent * mRedis;
	};
}


#endif //APP_DELAYMQCOMPONENT_H
