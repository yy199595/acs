#pragma once

#include"Util/Guid.h"
#include"Json/JsonWriter.h"
#include"Other/ElapsedTimer.h"
#include"RedisComponent.h"
#include"DB/Redis/RedisClientContext.h"
#include"Component/Coroutine/TaskComponent.h"

namespace Sentry
{
	class NetThreadComponent;

    class MainRedisComponent final : public RedisComponent, public ILuaRegister, public IStart
	{
	 public:
		MainRedisComponent() = default;
		~MainRedisComponent() final = default;

    public:
        template<typename ... Args>
        std::shared_ptr<RedisResponse> RunCmd(const std::string & name, const std::string & cmd, Args&& ... args);
        template<typename ... Args>
        std::shared_ptr<RedisResponse> RunCmd(SharedRedisClient redisClientContext, const std::string & cmd, Args&& ... args);

    public:
        bool Call(const std::string & name, const std::string & func, Json::Writer & request, std::shared_ptr<Json::Reader> response);
	 private:
        bool OnStart() final;
        bool LateAwake() final;
        void OnSecondUpdate(const int tick) final;
		void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) final;
	 private:
		TaskComponent* mTaskComponent;
		const struct RedisConfig* mConfig;
        std::vector<std::string> mTempArray;
	};

    template<typename ... Args>
    std::shared_ptr<RedisResponse> MainRedisComponent::RunCmd(const std::string &name, const std::string & cmd, Args &&...args)
    {
        std::shared_ptr<RedisRequest> request = std::make_shared<RedisRequest>(cmd);
        RedisRequest::InitParameter(request, std::forward<Args>(args)...);
        return this->Run(name, request);
    }
    template<typename ... Args>
    std::shared_ptr<RedisResponse> MainRedisComponent::RunCmd(SharedRedisClient redisClientContext, const std::string &cmd, Args &&...args)
    {
        std::shared_ptr<RedisRequest> request = std::make_shared<RedisRequest>(cmd);
        RedisRequest::InitParameter(request, std::forward<Args>(args)...);
        return this->Run(redisClientContext, request);
    }
}