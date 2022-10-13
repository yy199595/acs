#pragma once

#include"Guid/Guid.h"
#include"RedisComponent.h"
#include"Json/JsonWriter.h"
#include"Timer/ElapsedTimer.h"
#include"Client/TcpRedisClient.h"
#include"Component/TaskComponent.h"

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
        std::shared_ptr<RedisResponse> RunCommand(const std::string & name, const std::string & cmd, Args&& ... args);
        template<typename ... Args>
        std::shared_ptr<RedisResponse> RunCommand(TcpRedisClient * redisClientContext, const std::string & cmd, Args&& ... args);
    public:
        long long AddCounter(const std::string & id);
        long long SubCounter(const std::string & id);
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
    std::shared_ptr<RedisResponse> RedisDataComponent::RunCommand(const std::string &name, const std::string & cmd, Args &&...args)
    {
        std::shared_ptr<RedisRequest> request = std::make_shared<RedisRequest>(cmd);
        RedisRequest::InitParameter(request, std::forward<Args>(args)...);
        return this->Run(name, request);
    }
    template<typename ... Args>
    std::shared_ptr<RedisResponse> RedisDataComponent::RunCommand(TcpRedisClient * redisClientContext, const std::string &cmd, Args &&...args)
    {
        std::shared_ptr<RedisRequest> request = std::make_shared<RedisRequest>(cmd);
        RedisRequest::InitParameter(request, std::forward<Args>(args)...);
        return this->Run(redisClientContext, request);
    }
    template<typename ... Args>
    bool RedisDataComponent::SenCommond(const std::string &name, const std::string &cmd, Args &&...args)
    {
        TcpRedisClient * redisClient = this->GetClient(name);
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