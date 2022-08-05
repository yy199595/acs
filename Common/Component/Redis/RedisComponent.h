//
// Created by mac on 2022/5/18.
//

#ifndef SERVER_REDISBASECOMPONENT_H
#define SERVER_REDISBASECOMPONENT_H
#include"DB/Redis/RedisClientContext.h"
#include"Component/Rpc/RpcTaskComponent.h"

namespace Sentry
{
    class RedisLuaResponse
    {
    public:
        bool GetResult();
        bool ParseJson(const std::string & json);
        const Json::Reader & GetData() const { return this->mJson; }
    private:
        Json::Reader mJson;
    };
}

namespace Sentry
{
	struct RedisConfig;

    class RedisComponent : public RpcTaskComponent<RedisResponse>, public IStart
	{
	public:
		RedisComponent() = default;
	 protected:
        bool OnStart() final;
        bool LateAwake() final;
        SharedRedisClient MakeRedisClient(const RedisConfig & config);
        bool ParseConfig(const char * name, const rapidjson::Value & json);
    public:
        bool Ping(SharedRedisClient redisClient);
		virtual SharedRedisClient GetClient(const std::string & name);
        virtual void OnLoadScript(const std::string & name, const std::string & md5) { }
    protected:
        virtual bool OnInitRedisClient(RedisConfig config) = 0;
        std::shared_ptr<RedisResponse> Run(const std::string & name, std::shared_ptr<RedisRequest> request);
        std::shared_ptr<RedisResponse> Run(SharedRedisClient redisClientContext, std::shared_ptr<RedisRequest> request);
    private:
        class NetThreadComponent * mNetComponent;
        std::unordered_map<std::string, RedisConfig> mConfigs;
        std::unordered_map<std::string, std::vector<SharedRedisClient>> mRedisClients;
	};
}


#endif //SERVER_REDISBASECOMPONENT_H
