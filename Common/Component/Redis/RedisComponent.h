//
// Created by mac on 2022/5/18.
//

#ifndef SERVER_REDISBASECOMPONENT_H
#define SERVER_REDISBASECOMPONENT_H
#include"Component/Component.h"
#include"DB/Redis/RedisClientContext.h"
namespace Sentry
{
	struct RedisConfig;

	class RedisComponent : public Component, public IStart, public ISecondUpdate
	{
	public:
		RedisComponent() = default;
	public:
		bool Call(const std::string & name, const std::string& fullName, Json::Writer & jsonWriter);
        std::shared_ptr<RedisRequest> MakeLuaRequest(const std::string & fullName, const std::string & json);
		bool Call(const std::string & name,const std::string& fullName, Json::Writer & jsonWriter, std::shared_ptr<Json::Reader> response);
	 public:
        SharedRedisClient GetClient(const std::string & name);
        SharedRedisClient MakeRedisClient(const RedisConfig & config);
        SharedRedisClient MakeRedisClient(const std::string & name);
        bool TryAsyncConnect(SharedRedisClient client, int maxCount = 5);
        void OnResponse(SharedRedisClient client, std::shared_ptr<RedisResponse> response);
    public:
        std::shared_ptr<RedisResponse> Run(const std::string & name, std::shared_ptr<RedisRequest> request);
        std::shared_ptr<RedisResponse> Run(SharedRedisClient redisClientContext, std::shared_ptr<RedisRequest> request);
        template<typename ... Args>
        std::shared_ptr<RedisResponse> Run(const std::string & name, const std::string & cmd, Args&& ... args);
        template<typename ... Args>
        std::shared_ptr<RedisResponse> Run(SharedRedisClient redisClientContext, const std::string & cmd, Args&& ... args);
	protected:
		bool OnStart() override;
        bool LateAwake() override;
        void OnSecondUpdate(const int tick) override;
	 protected:
        virtual bool AddRedisTask(std::shared_ptr<IRpcTask<RedisResponse>> task) = 0;
        virtual void OnCommandReply(SharedRedisClient client, std::shared_ptr<RedisResponse> response) = 0;
        virtual void OnSubscribe(SharedRedisClient client, const std::string & channel, const std::string & message) = 0;
    private:
        void PushClient(SharedRedisClient redisClientContext);
        const RedisConfig * GetRedisConfig(const std::string & name);
		bool LoadLuaScript(SharedRedisClient redisClientContext, const std::string & path);
	private:
		TaskComponent * mTaskComponent;
		std::unordered_map<std::string, RedisConfig> mConfigs;
		std::unordered_map<std::string, std::string> mLuaMap;
		std::unordered_map<std::string, std::list<SharedRedisClient>> mRedisClients;
	};

    template<typename ... Args>
    std::shared_ptr<RedisResponse> RedisComponent::Run(const std::string &name, const std::string & cmd, Args &&...args)
    {
        std::shared_ptr<RedisRequest> request = std::make_shared<RedisRequest>(cmd);
        RedisRequest::InitParameter(request, std::forward<Args>(args)...);
        return this->Run(name, request);
    }
    template<typename ... Args>
    std::shared_ptr<RedisResponse> RedisComponent::Run(SharedRedisClient redisClientContext, const std::string &cmd, Args &&...args)
    {
        std::shared_ptr<RedisRequest> request = std::make_shared<RedisRequest>(cmd);
        RedisRequest::InitParameter(request, std::forward<Args>(args)...);
        return this->Run(redisClientContext, request);
    }
}


#endif //SERVER_REDISBASECOMPONENT_H
