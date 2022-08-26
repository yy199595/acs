#pragma once

#include"Util/Guid.h"
#include"Json/JsonWriter.h"
#include"Other/ElapsedTimer.h"
#include"RedisComponent.h"
#include"DB/Redis/TcpRedisClient.h"
#include"Component/Coroutine/TaskComponent.h"

namespace Sentry
{
	class NetThreadComponent;

    class RedisDataComponent final : public RedisComponent, public ILuaRegister
	{
	 public:
		RedisDataComponent() = default;
		~RedisDataComponent() final = default;
    public:
        template<typename ... Args>
        bool SenCommond(const std::string & name, const std::string & cmd, Args&& ... args);
        template<typename ... Args>
        std::shared_ptr<RedisResponse> RunCmd(const std::string & name, const std::string & cmd, Args&& ... args);
        template<typename ... Args>
        std::shared_ptr<RedisResponse> RunCmd(SharedRedisClient redisClientContext, const std::string & cmd, Args&& ... args);
    public:
        std::shared_ptr<RedisRequest> MakeLuaRequest(const std::string & fullName, const std::string & json);
        bool Call(const std::string & name, const std::string & func, Json::Writer & request, std::shared_ptr<Json::Reader> response);
	 private:
        bool OnInitRedisClient(RedisConfig config) final;
		void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) final;
        void OnLoadScript(const std::string & name, const std::string & md5) final;
    private:
		TaskComponent* mTaskComponent;
		const struct RedisConfig* mConfig;
        std::unordered_map<std::string, std::string> mLuaMap;

    };

    template<typename ... Args>
    std::shared_ptr<RedisResponse> RedisDataComponent::RunCmd(const std::string &name, const std::string & cmd, Args &&...args)
    {
        std::shared_ptr<RedisRequest> request = std::make_shared<RedisRequest>(cmd);
        RedisRequest::InitParameter(request, std::forward<Args>(args)...);
        return this->Run(name, request);
    }
    template<typename ... Args>
    std::shared_ptr<RedisResponse> RedisDataComponent::RunCmd(SharedRedisClient redisClientContext, const std::string &cmd, Args &&...args)
    {
        std::shared_ptr<RedisRequest> request = std::make_shared<RedisRequest>(cmd);
        RedisRequest::InitParameter(request, std::forward<Args>(args)...);
        return this->Run(redisClientContext, request);
    }
    template<typename ... Args>
    bool RedisDataComponent::SenCommond(const std::string &name, const std::string &cmd, Args &&...args)
    {
        SharedRedisClient redisClient = this->GetClient(name);
        if(redisClient == nullptr)
        {
            return false;
        }
        std::shared_ptr<RedisRequest> request = std::make_shared<RedisRequest>(cmd);
        RedisRequest::InitParameter(request, std::forward<Args>(args)...);
        redisClient->SendCommand(request);
        return true;
    }
}