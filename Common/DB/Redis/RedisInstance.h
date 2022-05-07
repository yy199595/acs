//
// Created by mac on 2022/5/6.
//

#ifndef SERVER_REDISINSTANCE_H
#define SERVER_REDISINSTANCE_H
#include"RedisClientContext.h"

namespace Sentry
{
	class RedisInstance
	{
	public:
		RedisInstance(const RedisConfig * config);
		~RedisInstance() = default;
		RedisInstance(const RedisInstance &) = delete;
	public:
		bool StartConnect();
		template<typename ... Args>
		std::shared_ptr<RedisResponse> Run(const std::string & cmd, Args && ... args);
	public:
		const std::string & GetName() { return this->mConfig->Name;}
		const std::string & GetAddress() { return this->mConfig->Address;}
	private:
		std::shared_ptr<RedisClientContext> GetRedisClient();
		std::shared_ptr<RedisClientContext> MakeRedisClient();
	private:
		const RedisConfig * mConfig;
		class TaskComponent * mTaskComponent;
		std::queue<std::shared_ptr<RedisClientContext>> mFreeClients;
	};

	template<typename ... Args>
	std::shared_ptr<RedisResponse> RedisInstance::Run(const std::string& cmd, Args && ... args)
	{
		std::shared_ptr<RedisClientContext> redisClientContext = this->GetRedisClient();
		return redisClientContext->Run(cmd, std::forward<Args>(args)...);
	}
}


#endif //SERVER_REDISINSTANCE_H
