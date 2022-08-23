//
// Created by mac on 2022/5/18.
//

#ifndef SERVER_REDISNODE_H
#define SERVER_REDISNODE_H
#include<vector>
#include<memory>
#include"TcpRedisClient.h"
namespace Sentry
{
	class RedisNode
	{
	public:
		RedisNode(const RedisConfig * config);
	public:
		std::shared_ptr<TcpRedisClient> GetFreeClient();
		void AddClient(std::shared_ptr<TcpRedisClient> client);
		const RedisConfig * GetConfig() const { return this->mConfig;}
	public:
		void CheckAllClient(long long nowTime);
	private:
		size_t mIndex;
		const RedisConfig * mConfig;
		std::vector<std::shared_ptr<TcpRedisClient>> mClients;
	};
}


#endif //SERVER_REDISNODE_H
