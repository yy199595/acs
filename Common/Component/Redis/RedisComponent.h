//
// Created by mac on 2022/5/18.
//

#ifndef SERVER_REDISBASECOMPONENT_H
#define SERVER_REDISBASECOMPONENT_H
#include"DB/Redis/RedisClientContext.h"
#include"Component/Rpc/RpcTaskComponent.h"
namespace Sentry
{
	struct RedisConfig;

	class RedisComponent : public RpcTaskComponent<RedisResponse>, public ISecondUpdate
	{
	public:
		RedisComponent() = default;
	protected:
        bool LateAwake() override;
        void OnSecondUpdate(const int tick) override;
	 protected:
        SharedRedisClient GetClient(const std::string & name);
        SharedRedisClient MakeRedisClient(const std::string &name);
        SharedRedisClient MakeRedisClient(const RedisConfig & config);
        const RedisConfig * ParseConfig(const char * name, const rapidjson::Value & json);
    public:
        virtual void OnLoadScript(const std::string & name, const std::string & md5) = 0;
    protected:
        std::shared_ptr<RedisResponse> Run(const std::string & name, std::shared_ptr<RedisRequest> request);
        std::shared_ptr<RedisResponse> Run(SharedRedisClient redisClientContext, std::shared_ptr<RedisRequest> request);
    private:
        void PushClient(SharedRedisClient redisClientContext);
        const RedisConfig * GetRedisConfig(const std::string & name);
    private:
		TaskComponent * mTaskComponent;
        class NetThreadComponent * mNetComponent;
		std::unordered_map<std::string, RedisConfig> mConfigs;
		std::unordered_map<std::string, std::list<SharedRedisClient>> mRedisClients;
	};
}


#endif //SERVER_REDISBASECOMPONENT_H
