//
// Created by mac on 2022/5/18.
//

#ifndef SERVER_REDISNODE_H
#define SERVER_REDISNODE_H
#include<vector>
#include<memory>
#include"RedisTcpClient.h"
namespace Tendo
{
	class RedisNode
	{
	public:
		RedisNode(const RedisClientConfig * config);
	public:
		std::shared_ptr<RedisTcpClient> GetFreeClient();
		void AddClient(std::shared_ptr<RedisTcpClient> client);
		const RedisClientConfig * GetConfig() const { return this->mConfig;}
	public:
		void CheckAllClient(long long nowTime);
	private:
		size_t mIndex;
		const RedisClientConfig * mConfig;
		std::vector<std::shared_ptr<RedisTcpClient>> mClients;
	};
}


#endif //SERVER_REDISNODE_H
