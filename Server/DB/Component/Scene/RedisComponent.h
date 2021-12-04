#pragma once

#include<Util/NumberHelper.h>
#include <Component/Component.h>
#include<RedisClient/RedisTaskProxy.h>
#include <Coroutine/CoroutineComponent.h>
using namespace GameKeeper;
namespace GameKeeper
{
	class TaskPoolComponent;

	class RedisComponent : public Component
	{
	public:
		RedisComponent();

		~RedisComponent() final = default;

    public:
        void ClearAllData();
		bool CloseRedisSocket();
		RedisSocket *GetRedisSocket();

	protected:
		bool Awake() final;            //初始化管理器
		void Start() override;                //初始化完成之后
	private:
		redisContext * ConnectRedis(int timeout = 3);
        std::shared_ptr<RedisResponse> StartTask(std::shared_ptr<RedisTaskProxy> task);
	public:
        long long AddCounter(const std::string & key);
        template<typename ... Args>
        std::shared_ptr<RedisResponse> Invoke(const std::string & cmd, Args &&...args)
        {
            std::shared_ptr<RedisTaskProxy> redisTask
                    = std::make_shared<RedisTaskProxy>(cmd);
            redisTask->InitCommand(std::forward<Args>(args) ...);
            return this->StartTask(redisTask);
        }
	private:
		std::string mRedisIp;        //redis ip地址
		unsigned short mRedisPort;    //端口号
        long long mLastOperatorTime;
		TaskPoolComponent *mTaskManager;
		CoroutineComponent *mCorComponent;
		std::unordered_map<std::thread::id, redisContext *> mRedisContextMap;
	};
}