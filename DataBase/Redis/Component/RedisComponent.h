//
// Created by mac on 2022/5/18.
//

#ifndef SERVER_REDISBASECOMPONENT_H
#define SERVER_REDISBASECOMPONENT_H
#include"Config/RedisConfig.h"
#include"Client/TcpRedisClient.h"
#include"Component/RpcTaskComponent.h"

namespace Sentry
{
    class RedisComponent : public RpcTaskComponent<RedisResponse>
	{
	public:
		RedisComponent() = default;
	 protected:
        virtual bool IsRunCommand() = 0;
        TcpRedisClient * MakeRedisClient(const RedisClientConfig & config);
    public:
        bool Ping(TcpRedisClient * redisClient);
		virtual TcpRedisClient * GetClient(const std::string & name);
        virtual void OnLoadScript(const std::string & name, const std::string & md5) { }
        std::shared_ptr<RedisResponse> Run(const std::string & name, std::shared_ptr<RedisRequest> request);
    protected:
        std::shared_ptr<RedisResponse> Run(TcpRedisClient * redisClientContext, std::shared_ptr<RedisRequest> request);
    private:
        std::unordered_map<std::string, std::vector<std::shared_ptr<TcpRedisClient>>> mRedisClients;
	};
}


#endif //SERVER_REDISBASECOMPONENT_H
