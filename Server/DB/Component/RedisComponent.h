#pragma once

#include"Util/Guid.h"
#include"hiredis/hiredis.h"
#include"Component/Component.h"
#include"Coroutine/TaskComponent.h"
#include"RedisClient/RedisTaskSource.h"
using namespace GameKeeper;
namespace GameKeeper
{
	class ThreadPoolComponent;

    class RedisComponent : public Component, public IStart
	{
	public:
		RedisComponent() = default;
		~RedisComponent() final = default;

    public:
        void ClearAllData();
		bool CloseRedisSocket();
		RedisSocket *GetRedisSocket();

	protected:
		bool Awake() final;            //初始化管理器
		bool LateAwake() final;                //初始化完成之后
        void OnStart() override;
	private:
		redisContext * ConnectRedis(int timeout = 3);
    private:
        void StartPubSub();
	public:
        long long AddCounter(const std::string & key);
        template<typename ... Args>
        std::shared_ptr<RedisResponse> Invoke(const std::string & cmd, Args &&...args)
        {
            std::shared_ptr<RedisTaskSource> redisTask
                    = std::make_shared<RedisTaskSource>(cmd);
            redisTask->InitCommand(std::forward<Args>(args) ...);
            return redisTask->Await();
        }
	private:
		std::string mRedisIp;        //redis ip地址
		unsigned short mRedisPort;    //端口号
        std::thread * mPubSubThread;
        redisContext * mPubSubContext;
		std::unordered_map<std::thread::id, redisContext *> mRedisContextMap;
	};
}