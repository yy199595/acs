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
		bool UnLock(const std::string& key); //分布式锁
		bool Lock(const std::string& key, int timeout = 10);
    public:
        bool Call(const std::string & name, const std::string& fullName, Json::Writer & jsonWriter);
        std::shared_ptr<RedisRequest> MakeLuaRequest(const std::string & fullName, const std::string & json);
        bool Call(const std::string & name,const std::string& fullName, Json::Writer & jsonWriter, std::shared_ptr<Json::Reader> response);
    public:
        template<typename ... Args>
        std::shared_ptr<RedisResponse> RunCmd(const std::string & name, const std::string & cmd, Args&& ... args);
        template<typename ... Args>
        std::shared_ptr<RedisResponse> RunCmd(SharedRedisClient redisClientContext, const std::string & cmd, Args&& ... args);

    protected:
        void OnLoadScript(const std::string &name, const std::string &md5) final;
	 private:
        bool OnStart() final;
        bool LateAwake() final;
        void OnSecondUpdate(const int tick) final;
		void OnLockTimeout(const std::string& name, int timeout);
		void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) final;
	 private:
		std::string mRpcAddress;
		TaskComponent* mTaskComponent;
		TimerComponent* mTimerComponent;
		const struct RedisConfig* mConfig;
		SharedRedisClient mSubRedisClient;
		class ServiceRpcComponent* mRpcTaskComponent;
        std::unordered_map<std::string, std::string> mLuaMap;
        std::unordered_map<std::string, long long> mLockTimers; //分布式锁的续命定时器
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